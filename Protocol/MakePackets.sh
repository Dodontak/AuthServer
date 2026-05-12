set -e
cd "$(dirname "$0")"

protoc --cpp_out=. Protocol.proto

./GenPackets --path=./Protocol.proto --output=ClientPacketHandler --recv=AC_ --send=AS_
./GenPackets --path=./Protocol.proto --output=ServerPacketHandler --recv=AS_ --send=AC_

cp Protocol.pb.cc Protocol.pb.h ../AuthServer
mv ClientPacketHandler.h ../AuthServer

cp Protocol.pb.cc Protocol.pb.h ../DummyClient
mv ServerPacketHandler.h ../DummyClient
