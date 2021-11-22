for f in $*
do
  echo
  echo "--- $f"
  name=$(basename -s .dat $f)
  sox --multi-threaded --show-progress --bits 32 --channels 1 --encoding floating-point --rate 16000 --endian little -t raw "$f" "$name.wav"
done
echo
