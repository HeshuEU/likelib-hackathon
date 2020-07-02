#include <boost/test/unit_test.hpp>

#include "base/bytes.hpp"
#include "base/hash.hpp"


BOOST_AUTO_TEST_CASE(sha256_hash)
{
    auto sha256_1 =
      base::Sha256::compute(base::Bytes{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30 });
    auto hex_str1 = "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846";
    BOOST_CHECK_EQUAL(sha256_1.toHex(), hex_str1);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(sha256_1.getBytes()), hex_str1);

    auto sha256_2 = base::Sha256::compute(base::Bytes("likelib.2"));
    auto hex_str2 = "1242fcfab7d240b6d6538dc0fa626cb2e43fa1186febd52cf4dce0da3c55a9e5";
    BOOST_CHECK_EQUAL(sha256_2.toHex(), hex_str2);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(sha256_2.getBytes()), hex_str2);

    auto sha256_3 = base::Sha256::compute(base::Bytes("it's third test"));
    auto hex_str3 = "2431f272555362f2d9ee255ec2ea24dffa371a03137699cf9d0b96e988346421";
    BOOST_CHECK_EQUAL(sha256_3.toHex(), hex_str3);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(sha256_3.getBytes()), hex_str3);

    auto sha256_4 = base::Sha256::compute(base::Bytes(""));
    auto hex_str4 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    BOOST_CHECK_EQUAL(sha256_4.toHex(), hex_str4);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(sha256_4.getBytes()), hex_str4);
}


BOOST_AUTO_TEST_CASE(sha256_serialization)
{
    auto target_hash =
      base::Sha256::compute(base::Bytes{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30 });
    auto target_hex_view = "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846";

    BOOST_CHECK_EQUAL(target_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(target_hash.getBytes()), target_hex_view);

    base::SerializationOArchive out;
    out.serialize(target_hash);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash = base::Sha256::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash, target_hash);
    BOOST_CHECK_EQUAL(deserialized_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(deserialized_hash.getBytes()),
                      target_hex_view);
}


BOOST_AUTO_TEST_CASE(sha256_hex)
{
    auto target_hash =
      base::Sha256::compute(base::Bytes{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30 });
    auto target_hex_view = "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846";

    auto hex_view = target_hash.toHex();
    auto from_hex_hash = base::Sha256::fromHex(hex_view);

    BOOST_CHECK_EQUAL(hex_view, target_hex_view);
    BOOST_CHECK_EQUAL(target_hash, from_hex_hash);
}


BOOST_AUTO_TEST_CASE(sha256_multiple_serialization)
{
    auto target_hash_1 =
      base::Sha256::compute(base::Bytes{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30 });
    auto target_hex_view_1 = "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846";

    BOOST_CHECK_EQUAL(target_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(target_hash_1.getBytes()), target_hex_view_1);

    auto target_hash_2 = base::Sha256::compute(base::Bytes("likelib.2"));
    auto target_hex_view_2 = "1242fcfab7d240b6d6538dc0fa626cb2e43fa1186febd52cf4dce0da3c55a9e5";

    BOOST_CHECK_EQUAL(target_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(target_hash_2.getBytes()), target_hex_view_2);

    base::SerializationOArchive out;
    out.serialize(target_hash_1);
    out.serialize(target_hash_2);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash_1 = base::Sha256::deserialize(in);
    auto deserialized_hash_2 = base::Sha256::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash_1, target_hash_1);
    BOOST_CHECK_EQUAL(deserialized_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(deserialized_hash_1.getBytes()),
                      target_hex_view_1);

    BOOST_CHECK_EQUAL(deserialized_hash_2, target_hash_2);
    BOOST_CHECK_EQUAL(deserialized_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha256::LENGTH>>(deserialized_hash_2.getBytes()),
                      target_hex_view_2);
}


BOOST_AUTO_TEST_CASE(sha1_hash)
{
    auto sha1_1 = base::Sha1::compute(base::Bytes{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30 });
    auto hex_str1 = "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08";
    BOOST_CHECK_EQUAL(sha1_1.toHex(), hex_str1);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(sha1_1.getBytes()), hex_str1);

    auto sha1_2 = base::Sha1::compute(base::Bytes("likelib.2"));
    auto hex_str2 = "ee2f5885c39b865f83e5f91dd94ce466f3be371d";
    BOOST_CHECK_EQUAL(sha1_2.toHex(), hex_str2);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(sha1_2.getBytes()), hex_str2);

    auto sha1_3 = base::Sha1::compute(base::Bytes("it's third test"));
    auto hex_str3 = "3f68e91144cc3d272df2950c0676918980a35d01";
    BOOST_CHECK_EQUAL(sha1_3.toHex(), hex_str3);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(sha1_3.getBytes()), hex_str3);

    auto sha1_4 = base::Sha1::compute(base::Bytes(""));
    auto hex_str4 = "da39a3ee5e6b4b0d3255bfef95601890afd80709";
    BOOST_CHECK_EQUAL(sha1_4.toHex(), hex_str4);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(sha1_4.getBytes()), hex_str4);
}


BOOST_AUTO_TEST_CASE(sha1_serialization)
{
    auto target_hash =
      base::Sha1::compute(base::Bytes{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30 });
    auto target_hex_view = "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08";

    BOOST_CHECK_EQUAL(target_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(target_hash.getBytes()), target_hex_view);

    base::SerializationOArchive out;
    out.serialize(target_hash);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash = base::Sha1::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash, target_hash);
    BOOST_CHECK_EQUAL(deserialized_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(deserialized_hash.getBytes()), target_hex_view);
}


BOOST_AUTO_TEST_CASE(sha1_hex)
{
    auto target_hash =
      base::Sha1::compute(base::Bytes{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30 });
    auto target_hex_view = "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08";

    auto hex_view = target_hash.toHex();
    auto from_hex_hash = base::Sha1::fromHex(hex_view);

    BOOST_CHECK_EQUAL(hex_view, target_hex_view);
    BOOST_CHECK_EQUAL(target_hash, from_hex_hash);
}


BOOST_AUTO_TEST_CASE(sha1_multiple_serialization)
{
    auto target_hash_1 =
      base::Sha1::compute(base::Bytes{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30 });
    auto target_hex_view_1 = "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08";

    BOOST_CHECK_EQUAL(target_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(target_hash_1.getBytes()), target_hex_view_1);

    auto target_hash_2 = base::Sha1::compute(base::Bytes("likelib.2"));
    auto target_hex_view_2 = "ee2f5885c39b865f83e5f91dd94ce466f3be371d";

    BOOST_CHECK_EQUAL(target_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(target_hash_2.getBytes()), target_hex_view_2);

    base::SerializationOArchive out;
    out.serialize(target_hash_1);
    out.serialize(target_hash_2);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash_1 = base::Sha1::deserialize(in);
    auto deserialized_hash_2 = base::Sha1::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash_1, target_hash_1);
    BOOST_CHECK_EQUAL(deserialized_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(deserialized_hash_1.getBytes()),
                      target_hex_view_1);

    BOOST_CHECK_EQUAL(deserialized_hash_2, target_hash_2);
    BOOST_CHECK_EQUAL(deserialized_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Sha1::LENGTH>>(deserialized_hash_2.getBytes()),
                      target_hex_view_2);
}


BOOST_AUTO_TEST_CASE(ripemd160_hash)
{
    auto target_hash_1 = base::Ripemd160::compute(base::Bytes("likelib.2"));
    auto hex_str2 = "cd5cbbaf134e907e8ba58a3fe462b1d48e4157ea";
    BOOST_CHECK_EQUAL(target_hash_1.toHex(), hex_str2);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Ripemd160::LENGTH>>(target_hash_1.getBytes()), hex_str2);

    auto target_hash_2 = base::Ripemd160::compute(base::Bytes("it's third test"));
    auto hex_str3 = "5db1d7ced17f628908569b7933e4c9aa69ccebf4";
    BOOST_CHECK_EQUAL(target_hash_2.toHex(), hex_str3);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Ripemd160::LENGTH>>(target_hash_2.getBytes()), hex_str3);

    auto target_hash_3 = base::Ripemd160::compute(base::Bytes(""));
    auto hex_str4 = "9c1185a5c5e9fc54612808977ee8f548b2258d31";
    BOOST_CHECK_EQUAL(target_hash_3.toHex(), hex_str4);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Ripemd160::LENGTH>>(target_hash_3.getBytes()), hex_str4);
}


BOOST_AUTO_TEST_CASE(ripemd160_serialization)
{
    auto target_hash = base::Ripemd160::compute(base::Bytes{ "likelib.2" });
    auto target_hex_view = "cd5cbbaf134e907e8ba58a3fe462b1d48e4157ea";

    BOOST_CHECK_EQUAL(target_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Ripemd160::LENGTH>>(target_hash.getBytes()), target_hex_view);

    base::SerializationOArchive out;
    out.serialize(target_hash);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash = base::Ripemd160::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash, target_hash);
    BOOST_CHECK_EQUAL(deserialized_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Ripemd160::LENGTH>>(deserialized_hash.getBytes()),
                      target_hex_view);
}


BOOST_AUTO_TEST_CASE(ripemd160_hex)
{
    auto target_hash = base::Ripemd160::compute(base::Bytes{ "likelib.2" });
    auto target_hex_view = "cd5cbbaf134e907e8ba58a3fe462b1d48e4157ea";

    auto hex_view = target_hash.toHex();
    auto from_hex_hash = base::Ripemd160::fromHex(hex_view);

    BOOST_CHECK_EQUAL(hex_view, target_hex_view);
    BOOST_CHECK_EQUAL(target_hash, from_hex_hash);
}


BOOST_AUTO_TEST_CASE(ripemd160_multiple_serialization)
{
    auto target_hash_1 = base::Ripemd160::compute(base::Bytes{ "" });
    auto target_hex_view_1 = "9c1185a5c5e9fc54612808977ee8f548b2258d31";

    BOOST_CHECK_EQUAL(target_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Ripemd160::LENGTH>>(target_hash_1.getBytes()),
                      target_hex_view_1);

    auto target_hash_2 = base::Ripemd160::compute(base::Bytes("likelib.2"));
    auto target_hex_view_2 = "cd5cbbaf134e907e8ba58a3fe462b1d48e4157ea";

    BOOST_CHECK_EQUAL(target_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Ripemd160::LENGTH>>(target_hash_2.getBytes()),
                      target_hex_view_2);

    base::SerializationOArchive out;
    out.serialize(target_hash_1);
    out.serialize(target_hash_2);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash_1 = base::Ripemd160::deserialize(in);
    auto deserialized_hash_2 = base::Ripemd160::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash_1, target_hash_1);
    BOOST_CHECK_EQUAL(deserialized_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Ripemd160::LENGTH>>(deserialized_hash_1.getBytes()),
                      target_hex_view_1);

    BOOST_CHECK_EQUAL(deserialized_hash_2, target_hash_2);
    BOOST_CHECK_EQUAL(deserialized_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<base::Ripemd160::LENGTH>>(deserialized_hash_2.getBytes()),
                      target_hex_view_2);
}


BOOST_AUTO_TEST_CASE(sha3_hash)
{
    auto sha224 = base::Sha3::compute(base::Bytes("3-19Lg3;)3;"), base::Sha3::Sha3Type::Sha3Type224);
    auto hex_str1 = "415cb72b4679aabbce2666574fc0040e9451366713fb51bf06f5a861";
    BOOST_CHECK_EQUAL(sha224.toHex(), hex_str1);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(sha224.getBytes()), hex_str1);

    auto sha256 = base::Sha3::compute(base::Bytes("395sdojtse"), base::Sha3::Sha3Type::Sha3Type256);
    auto hex_str2 = "83118af0614d2afc3d0ffe1f1ffbb12f1f971ad293c96e2bc6e710cccf65b004";
    BOOST_CHECK_EQUAL(sha256.toHex(), hex_str2);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(sha256.getBytes()), hex_str2);

    auto sha384 = base::Sha3::compute(base::Bytes("fJ#05M30fc3;"), base::Sha3::Sha3Type::Sha3Type384);
    auto hex_str3 = "574efa5b544ecc526a9e2d024747eb2a7acc9cb70b404c8f68cde89fd079d1d7b068691b55abe88a7f07f87f4fe682f3";
    BOOST_CHECK_EQUAL(sha384.toHex(), hex_str3);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(sha384.getBytes()), hex_str3);

    auto sha512 = base::Sha3::compute(base::Bytes("0L35;'fKw35p0Gk"), base::Sha3::Sha3Type::Sha3Type512);
    auto hex_str4 =
      "5132a80abfbcdcea658ec25f1e84886b3771dc5dc247ed71bb9660901ef71038475179d5b420d2cfd5e5d1eaf1b24ea7f46ace5bc676713d2092cc5123d3f835";
    BOOST_CHECK_EQUAL(sha512.toHex(), hex_str4);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(sha512.getBytes()), hex_str4);
}


BOOST_AUTO_TEST_CASE(sha3_hash_empty)
{
    auto sha224 = base::Sha3::compute(base::Bytes(""), base::Sha3::Sha3Type::Sha3Type224);
    auto hex_str1 = "6b4e03423667dbb73b6e15454f0eb1abd4597f9a1b078e3f5b5a6bc7";
    BOOST_CHECK_EQUAL(sha224.toHex(), hex_str1);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(sha224.getBytes()), hex_str1);

    auto sha256 = base::Sha3::compute(base::Bytes(""), base::Sha3::Sha3Type::Sha3Type256);
    auto hex_str2 = "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a";
    BOOST_CHECK_EQUAL(sha256.toHex(), hex_str2);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(sha256.getBytes()), hex_str2);

    auto sha384 = base::Sha3::compute(base::Bytes(""), base::Sha3::Sha3Type::Sha3Type384);
    auto hex_str3 = "0c63a75b845e4f7d01107d852e4c2485c51a50aaaa94fc61995e71bbee983a2ac3713831264adb47fb6bd1e058d5f004";
    BOOST_CHECK_EQUAL(sha384.toHex(), hex_str3);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(sha384.getBytes()), hex_str3);

    auto sha512 = base::Sha3::compute(base::Bytes(""), base::Sha3::Sha3Type::Sha3Type512);
    auto hex_str4 =
      "a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26";
    BOOST_CHECK_EQUAL(sha512.toHex(), hex_str4);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(sha512.getBytes()), hex_str4);
}


BOOST_AUTO_TEST_CASE(sha3_serialization)
{
    auto target_hash = base::Sha3::compute(base::Bytes{ "likelib.2" }, base::Sha3::Sha3Type::Sha3Type256);
    auto target_hex_view = "62e3a6a603eb661780d4a48b21349070b7003fae7125deec536ed35f04f33ef1";

    BOOST_CHECK_EQUAL(target_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(target_hash.getBytes()), target_hex_view);

    base::SerializationOArchive out;
    out.serialize(target_hash);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash = base::Sha3::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash, target_hash);
    BOOST_CHECK_EQUAL(deserialized_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(deserialized_hash.getBytes()), target_hex_view);
}


BOOST_AUTO_TEST_CASE(sha3_hex)
{
    auto target_hash = base::Sha3::compute(base::Bytes{ "likelib.2" }, base::Sha3::Sha3Type::Sha3Type256);
    auto target_hex_view = "62e3a6a603eb661780d4a48b21349070b7003fae7125deec536ed35f04f33ef1";

    auto hex_view = target_hash.toHex();
    auto from_hex_hash = base::Sha3::fromHex(hex_view);

    BOOST_CHECK_EQUAL(hex_view, target_hex_view);
    BOOST_CHECK_EQUAL(target_hash, from_hex_hash);
}


BOOST_AUTO_TEST_CASE(sha3_multiple_serialization)
{
    auto target_hash_1 = base::Sha3::compute(base::Bytes(""), base::Sha3::Sha3Type::Sha3Type256);
    auto target_hex_view_1 = "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a";

    BOOST_CHECK_EQUAL(target_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(target_hash_1.getBytes()), target_hex_view_1);

    auto target_hash_2 = base::Sha3::compute(base::Bytes{ "likelib.2" }, base::Sha3::Sha3Type::Sha3Type256);
    auto target_hex_view_2 = "62e3a6a603eb661780d4a48b21349070b7003fae7125deec536ed35f04f33ef1";

    BOOST_CHECK_EQUAL(target_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(target_hash_2.getBytes()), target_hex_view_2);

    base::SerializationOArchive out;
    out.serialize(target_hash_1);
    out.serialize(target_hash_2);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash_1 = base::Sha3::deserialize(in);
    auto deserialized_hash_2 = base::Sha3::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash_1, target_hash_1);
    BOOST_CHECK_EQUAL(deserialized_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(deserialized_hash_1.getBytes()), target_hex_view_1);

    BOOST_CHECK_EQUAL(deserialized_hash_2, target_hash_2);
    BOOST_CHECK_EQUAL(deserialized_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(deserialized_hash_2.getBytes()), target_hex_view_2);
}