#!/bin/bash
BASE_URI="http://10.8.0.1/fpga_bitstreams/"
VERSIONS_TO_FETCH=( A10 A12 A14 A15 A16 A17)
SCRIPT_PATH=`pwd`/`dirname $0`
BITSTREAM_PATH=${SCRIPT_PATH}/include/bitstream.h
BITSTREAM_URI_PREFIX="SmartScope_"
BITSTREAM_URI_SUFFIX=".bin"

rm ${BITSTREAM_PATH}
for i in "${VERSIONS_TO_FETCH[@]}"; do
  BITSTREAM_URI="${BASE_URI}${i}/${BITSTREAM_URI_PREFIX}${i}${BITSTREAM_FILE_SUFFIX}"
  echo Fetching ${BITSTREAM_URI} TO ${BITSTREAM_FILENAME}
  echo unsigned char smartscope_fpga_$i[] = { >> ${BITSTREAM_PATH}
  curl ${BITSTREAM_URI} -# | xxd -i >> ${BITSTREAM_PATH}
  echo }\; >> ${BITSTREAM_PATH} 
done
