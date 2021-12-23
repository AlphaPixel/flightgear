export CAPTURE_FILE=$1

echo "Reading $CAPTURE_FILE"

# Get the main interface name (which will be the one we'll use to as the source of the replay packets)
export DEST_IFACE=$(ip route get 1.1.1.1 | grep -Po '(?<=dev\s)\w+' | cut -f1 -d ' ')
export DEST_MAC=$(ifconfig $DEST_IFACE | awk '/ether /{print $2}')

# Determine the source MAC to replace
export SRC_MAC=$(tshark -r "$CAPTURE_FILE" -O DIS -T fields -E separator=, -e eth.src -c 1)

echo "MAC address mapping:  $SRC_MAC -> $DEST_MAC"

export OUTPUT_FILE=$(dirname "$CAPTURE_FILE")/$(basename "$CAPTURE_FILE" .pcapng).rewrite.pcapng

echo "Creating $OUTPUT_FILE"
tcprewrite --enet-subsmac=$SRC_MAC,$DEST_MAC --infile="$CAPTURE_FILE" --fixcsum --outfile="$OUTPUT_FILE"
