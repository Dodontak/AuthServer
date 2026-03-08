set -e
cd "$(dirname "$0")"

protoc --cpp_out=. Protocol.proto

./GenPackets --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
./GenPackets --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_

cp Protocol.pb.cc Protocol.pb.h ../AuthServer
mv ClientPacketHandler.h ../AuthServer

cp Protocol.pb.cc Protocol.pb.h ../DummyClient
mv ServerPacketHandler.h ../DummyClient
