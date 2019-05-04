# test 1
mkdir -p baseDir
rm baseDir/* 2> /dev/null
bin/server > testcases/test1_server.out &
bin/client 127.0.0.1 1337 testcases/test1.in testcases/test1_client.out
echo
echo "***** Test 1 client output *****"
cat testcases/test1_client.out
echo
echo "***** Test 1 server output *****"
cat testcases/test1_server.out


# test 3
mkdir -p baseDir
rm baseDir/* 2> /dev/null
bin/server > testcases/test3_server.out &
bin/client 127.0.0.1 1337 testcases/test3.in testcases/test3_client.out
echo
echo "***** Test 3 client output *****"
cat testcases/test3_client.out
echo
echo "***** Test 3 server output *****"
cat testcases/test3_server.out

# test 4
mkdir -p baseDir
rm baseDir/* 2> /dev/null
bin/server > testcases/test4_server.out &
bin/client 127.0.0.1 1337 testcases/test4.in testcases/test4_client.out
echo
echo "***** Test 3 client output *****"
cat testcases/test4_client.out
echo
echo "***** Test 3 server output *****"
cat testcases/test4_server.out
