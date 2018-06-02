#include "QmlAircraftInfo.hxx"

#include <QVariant>
#include <QDebug>
#include <QQmlEngine>
#include <QQmlComponent>

#include <simgear/package/Install.hxx>
#include <simgear/package/Root.hxx>
#include <simgear/structure/exception.hxx>
#include <simgear/props/props_io.hxx>

#include <Include/version.h>
#include <Main/globals.hxx>

#include "LocalAircraftCache.hxx"

using namespace simgear::pkg;

const int QmlAircraftInfo::StateTagRole =  Qt::UserRole + 1;
const int QmlAircraftInfo::StateDescriptionRole = Qt::UserRole + 2;
const int QmlAircraftInfo::StateExplicitRole = Qt::UserRole + 3;

class QmlAircraftInfo::Delegate : public simgear::pkg::Delegate
{
public:
    Delegate(QmlAircraftInfo* info) :
        p(info)
    {
        globals->packageRoot()->addDelegate(this);
    }

    ~Delegate()
    {
        globals->packageRoot()->removeDelegate(this);
    }

protected:
    void catalogRefreshed(CatalogRef, StatusCode) override
    {
    }

    void startInstall(InstallRef aInstall) override
    {
        if (aInstall->package() == p->packageRef()) {
            p->setDownloadBytes(0);
        }
    }

    void installProgress(InstallRef aInstall, unsigned int bytes, unsigned int total) override
    {
        if (aInstall->package() == p->packageRef()) {
            p->setDownloadBytes(bytes);
        }
    }

    void finishInstall(InstallRef aInstall, StatusCode aReason) override
    {
        Q_UNUSED(aReason);
        if (aInstall->package() == p->packageRef()) {
            p->checkForStates();
            p->infoChanged();
        }
    }

    void installStatusChanged(InstallRef aInstall, StatusCode aReason) override
    {
        Q_UNUSED(aReason);
        if (aInstall->package() == p->packageRef()) {
            p->downloadChanged();
        }
    }

    void finishUninstall(const PackageRef& pkg) override
    {
        if (pkg == p->packageRef()) {
            p->downloadChanged();
        }
    }

private:

    QmlAircraftInfo* p;
};

////////////////////////////////////////////////////////////////////////////


struct StateInfo
{
    string_list tags;
    QString name;   // human-readable name, or blank if we auto-generate this
    QString description; // human-readable description

    std::string primaryTag() const
    { return tags.front(); }
};

using AircraftStateVec = std::vector<StateInfo>;

static AircraftStateVec readAircraftStates(const SGPath& setXMLPath)
{
    SGPropertyNode_ptr root(new SGPropertyNode);
    try {
        readProperties(setXMLPath, root);
    } catch (sg_exception&) {
        // malformed include or XML, just bail
        return {};
    }
    
    if (!root->getNode("sim/state")) {
        return {};
    }

    auto nodes = root->getNode("sim")->getChildren("state");
    AircraftStateVec result;
    result.reserve(nodes.size());
    for (auto cn : nodes) {
        string_list stateNames;
        for (auto nameNode : cn->getChildren("name")) {
            stateNames.push_back(nameNode->getStringValue());
        }

        if (stateNames.empty()) {
            qWarning() << "state with no names defined, skipping" << QString::fromStdString(cn->getPath());
            continue;
        }

        result.push_back({stateNames,
                          QString::fromStdString(cn->getStringValue("readable-name")),
                          QString::fromStdString(cn->getStringValue("description"))
                         });
    }

    return result;
}

QString humanNameFromStateTag(const std::string& tag)
{
    if (tag == "approach") return QObject::tr("On approach");
    if (tag == "take-off") return QObject::tr("Ready for Take-off");
    if ((tag == "parking") || (tag == "cold-and-dark"))
        return QObject::tr("Parked, cold & dark");
    if (tag == "auto")
        return QObject::tr("Automatic");
    if (tag == "cruise")
        return QObject::tr("Cruise");

    qWarning() << Q_FUNC_INFO << "add for" << QString::fromStdString(tag);
    // no mapping, let's use the tag directly
    return QString::fromStdString(tag);
}

class StatesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool hasExplicitAuto READ hasExplicitAuto CONSTANT)
public:
    StatesModel()
    {
    }

    StatesModel(const AircraftStateVec& states) :
        _data(states)
    {
        // we use an empty model for aircraft with no states defined
        if (states.empty()) {
            return;
        }

        // sort which places 'auto' item at the front if it exists
        std::sort(_data.begin(), _data.end(), [](const StateInfo& a, const StateInfo& b) {
            if (a.primaryTag() == "auto") return true;
            if (b.primaryTag() == "auto") return false;
            return a.primaryTag() < b.primaryTag();
        });

        if (_data.front().primaryTag() == "auto") {
            // track if the aircraft supplied an 'auto' state, in which case
            // we will not run our own selection logic
            _explicitAutoState = true;
        } else {
            // disabling this code for 2018.1, since it needs more testing
            _data.insert(_data.begin(), {{"auto"}, {}, tr("Select state based on startup position.")});
        }
    }

    int indexForTag(const std::string &tag) const
    {
        auto it = std::find_if(_data.begin(), _data.end(), [tag](const StateInfo& i) {
            auto tagIt = std::find(i.tags.begin(), i.tags.end(), tag);
            return (tagIt != i.tags.end());
        });

        if (it == _data.end())
            return -1;

        return std::distance(_data.begin(), it);
    }

    int rowCount(const QModelIndex &parent) const override
    {
        return _data.size();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        const StateInfo& s = _data.at(index.row());
        if (role == Qt::DisplayRole) {
            if (s.name.isEmpty()) {
                return humanNameFromStateTag(s.primaryTag());
            }
            return s.name;
        } else if (role == QmlAircraftInfo::StateTagRole) {
            return QString::fromStdString(s.primaryTag());
        } else if (role == QmlAircraftInfo::StateDescriptionRole) {
            return s.description;
        } else if (role == QmlAircraftInfo::StateExplicitRole) {
            if (s.primaryTag() == "auto")
                return _explicitAutoState;
            return true;
        }

        return {};
    }

    QHash<int, QByteArray> roleNames() const override
    {
        auto result = QAbstractListModel::roleNames();
        result[Qt::DisplayRole] = "name";
        result[QmlAircraftInfo::StateTagRole] = "tag";
        result[QmlAircraftInfo::StateDescriptionRole] = "description";
        return result;
    }

    Q_INVOKABLE QString descriptionForState(int row) const
    {
        if ((row < 0) || (row >= _data.size()))
            return {};

        const StateInfo& s = _data.at(row);
        return s.description;
    }

    Q_INVOKABLE QString tagForState(int row) const
    {
        if ((row < 0) || (row >= _data.size()))
            return {};

        return QString::fromStdString(_data.at(row).primaryTag());
    }

    bool hasExplicitAuto() const
    {
        return _explicitAutoState;
    }

    bool isEmpty() const
    {
        return _data.empty();
    }

    bool hasState(QString st) const
    {
        return indexForTag(st.toStdString()) != -1;
    }
private:
    AircraftStateVec _data;
    bool _explicitAutoState = false;
};

////////////////////////////////////////////////////////////////////////////

QmlAircraftInfo::QmlAircraftInfo(QObject *parent)
    : QObject(parent)
    , _delegate(new Delegate(this))
{
    qmlRegisterUncreatableType<StatesModel>("FlightGear.Launcher", 1, 0, "StatesModel", "no");
}

QmlAircraftInfo::~QmlAircraftInfo()
{

}

QUrl QmlAircraftInfo::uri() const
{
    if (_item) {
        return QUrl::fromLocalFile(resolveItem()->path);
    } else if (_package) {
        return QUrl("package:" + QString::fromStdString(_package->qualifiedVariantId(_variant)));
    }

    return {};
}

int QmlAircraftInfo::numVariants() const
{
    if (_item) {
        // for on-disk, we don't count the primary item
        return _item->variants.size() + 1;
    } else if (_package) {
        // whereas for packaged aircraft we do
        return _package->variants().size();
    }

    return 0;
}

QString QmlAircraftInfo::name() const
{
    if (_item) {
        return resolveItem()->description;
    } else if (_package) {
        return QString::fromStdString(_package->nameForVariant(_variant));
    }

    return {};
}

QString QmlAircraftInfo::description() const
{
    if (_item) {
        return resolveItem()->longDescription;
    } else if (_package) {
        std::string longDesc = _package->getLocalisedProp("description", _variant);
        return QString::fromStdString(longDesc).simplified();
    }

    return {};
}

QString QmlAircraftInfo::authors() const
{
    if (_item) {
        return resolveItem()->authors;
    } else if (_package) {
        std::string authors = _package->getLocalisedProp("author", _variant);
        return QString::fromStdString(authors);
    }

    return {};
}

QVariantList QmlAircraftInfo::ratings() const
{
    if (_item) {
        QVariantList result;
        auto actualItem = resolveItem();
        for (int i=0; i<4; ++i) {
            result << actualItem->ratings[i];
        }
        return result;
    } else if (_package) {
        SGPropertyNode* ratings = _package->properties()->getChild("rating");
        if (!ratings) {
            return {};
        }

        QVariantList result;
        for (int i=0; i<4; ++i) {
            result << ratings->getChild(i)->getIntValue();
        }
        return result;
    }
    return {};
}

QVariantList QmlAircraftInfo::previews() const
{
    if (_item) {
        QVariantList result;
        auto actualItem = resolveItem();
        Q_FOREACH(QUrl u, actualItem->previews) {
            result.append(u);
        }
        return result;
    }

    if (_package) {
        const auto& previews = _package->previewsForVariant(_variant);
        if (previews.empty()) {
            return {};
        }

        QVariantList result;
        // if we have an install, return file URLs, not remote (http) ones
        auto ex = _package->existingInstall();
        if (ex.valid()) {
            for (auto p : previews) {
                SGPath localPreviewPath = ex->path() / p.path;
                if (!localPreviewPath.exists()) {
                    qWarning() << "missing local preview" << QString::fromStdString(localPreviewPath.utf8Str());
                    continue;
                }
                result.append(QUrl::fromLocalFile(QString::fromStdString(localPreviewPath.utf8Str())));
            }
            return result;
        }

        // return remote urls
        for (auto p : previews) {
            result.append(QUrl(QString::fromStdString(p.url)));
        }

        return result;
    }

    return {};
}

QUrl QmlAircraftInfo::thumbnail() const
{
    if (_item) {
        return QUrl::fromLocalFile(resolveItem()->thumbnailPath);
    } else if (_package) {
        auto t = _package->thumbnailForVariant(_variant);
        if (QFileInfo::exists(QString::fromStdString(t.path))) {
            return QUrl::fromLocalFile(QString::fromStdString(t.path));
        }
        return QUrl(QString::fromStdString(t.url));
    }

    return {};
}

QString QmlAircraftInfo::pathOnDisk() const
{
    if (_item) {
        return resolveItem()->path;
    } else if (_package) {
        auto install = _package->existingInstall();
        if (install.valid()) {
            return QString::fromStdString(install->primarySetPath().utf8Str());
        }
    }

    return {};
}

QString QmlAircraftInfo::packageId() const
{
    if (_package) {
        return QString::fromStdString(_package->variants()[_variant]);
    }

    return {};
}

int QmlAircraftInfo::packageSize() const
{
    if (_package) {
        return _package->fileSizeBytes();
    }

    return 0;
}

int QmlAircraftInfo::downloadedBytes() const
{
    return _downloadBytes;
}

QVariant QmlAircraftInfo::status() const
{
    if (_item) {
        return _item->status(_variant);
    } else if (_package) {
        return packageAircraftStatus(_package);
    }

    return LocalAircraftCache::AircraftOk;
}

QString QmlAircraftInfo::minimumFGVersion() const
{
    if (_item) {
        return resolveItem()->minFGVersion;
    } else if (_package) {
        const std::string v = _package->properties()->getStringValue("minimum-fg-version");
        if (!v.empty()) {
            return QString::fromStdString(v);
        }
    }

    return {};
}

AircraftItemPtr QmlAircraftInfo::resolveItem() const
{
    if (_variant > 0) {
        return _item->variants.at(_variant - 1);
    }

    return _item;
}

void QmlAircraftInfo::checkForStates()
{
    QString path = pathOnDisk();
    if (path.isEmpty()) {
        _statesModel.reset(new StatesModel);
        emit infoChanged();
        return;
    }

    auto states = readAircraftStates(SGPath::fromUtf8(path.toUtf8().toStdString()));
    if (states.empty()) {
        _statesModel.reset(new StatesModel);
        emit infoChanged();
        return;
    }

    _statesModel.reset(new StatesModel(states));
    emit infoChanged();
}

void QmlAircraftInfo::setUri(QUrl u)
{
    if (uri() == u)
        return;

    _item.clear();
    _package.clear();
    _statesModel.reset(new StatesModel);

    if (u.isLocalFile()) {
        _item = LocalAircraftCache::instance()->findItemWithUri(u);
        if (!_item) {
            // scan still active or aircraft not found, let's bail out
            // and rely on caller to try again
            return;
        }

        int vindex = _item->indexOfVariant(u);
        // we need to offset the variant index to allow for the different
        // indexing schemes here (primary included) and in the cache (primary
        // is not counted)
        _variant = (vindex >= 0) ? vindex + 1 : 0;
    } else if (u.scheme() == "package") {
        auto ident = u.path().toStdString();
        try {
            _package = globals->packageRoot()->getPackageById(ident);
            if (_package) {
                _variant = _package->indexOfVariant(ident);
            }
        } catch (sg_exception&) {
            qWarning() << "couldn't find package/variant for " << u;
        }
    }

    checkForStates();

    emit uriChanged();
    emit infoChanged();
    emit downloadChanged();
}

void QmlAircraftInfo::setVariant(int variant)
{
    if (!_item && !_package)
        return;

    if ((variant < 0) || (variant >= numVariants())) {
        qWarning() << Q_FUNC_INFO << uri() << "variant index out of range:" << variant;
        return;
    }

    if (_variant == variant)
        return;

    _variant = variant;
    checkForStates();

    emit infoChanged();
    emit variantChanged(_variant);
}

void QmlAircraftInfo::requestInstallUpdate()
{

}

void QmlAircraftInfo::requestUninstall()
{

}

void QmlAircraftInfo::requestInstallCancel()
{

}

QVariant QmlAircraftInfo::packageAircraftStatus(simgear::pkg::PackageRef p)
{
    if (p->hasTag("needs-maintenance")) {
        return LocalAircraftCache::AircraftUnmaintained;
    }

    if (!p->properties()->hasChild("minimum-fg-version")) {
        return LocalAircraftCache::AircraftOk;
    }

    const std::string minFGVersion = p->properties()->getStringValue("minimum-fg-version");
    const int c = simgear::strutils::compare_versions(FLIGHTGEAR_VERSION, minFGVersion, 2);
    return (c < 0) ? LocalAircraftCache::AircraftNeedsNewerSimulator :
                     LocalAircraftCache::AircraftOk;
}

QVariant QmlAircraftInfo::installStatus() const
{
    if (_item) {
        return LocalAircraftCache::PackageInstalled;
    }

    if (_package) {
        auto i = _package->existingInstall();
        if (i.valid()) {
            if (i->isDownloading()) {
                return LocalAircraftCache::PackageDownloading;
            }
            if (i->isQueued()) {
                return LocalAircraftCache::PackageQueued;
            }
            if (i->hasUpdate()) {
                return LocalAircraftCache::PackageUpdateAvailable;
            }

            return LocalAircraftCache::PackageInstalled;
        } else {
            return LocalAircraftCache::PackageNotInstalled;
        }
    }

    return LocalAircraftCache::NotPackaged;
}

PackageRef QmlAircraftInfo::packageRef() const
{
    return _package;
}

void QmlAircraftInfo::setDownloadBytes(int bytes)
{
    _downloadBytes = bytes;
    emit downloadChanged();
}

QStringList QmlAircraftInfo::variantNames() const
{
    QStringList result;
    if (_item) {
        result.append(_item->description);
        Q_FOREACH(auto v, _item->variants) {
            if (v->description.isEmpty()) {
                qWarning() << Q_FUNC_INFO << "missing description for " << v->path;
            }
            result.append(v->description);
        }
    } else if (_package) {
        for (int vindex = 0; vindex < _package->variants().size(); ++vindex) {
            if (_package->nameForVariant(vindex).empty()) {
                qWarning() << Q_FUNC_INFO << "missing description for variant" << vindex;
            }
            result.append(QString::fromStdString(_package->nameForVariant(vindex)));
        }
    }
    return result;
}

bool QmlAircraftInfo::isPackaged() const
{
    return _package != PackageRef();
}

bool QmlAircraftInfo::hasStates() const
{
    return !_statesModel->isEmpty();
}

bool QmlAircraftInfo::hasState(QString name) const
{
    return _statesModel->hasState(name);
}

StatesModel *QmlAircraftInfo::statesModel()
{
    return _statesModel.data();
}

#include "QmlAircraftInfo.moc"

