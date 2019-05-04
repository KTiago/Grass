for i in 1 3 4 5 6 7 8 #9
do 
        mkdir -p baseDir
        rm baseDir/* 2> /dev/null
        if [[("$i"=="4")]];
        then
                touch baseDir/server_file.txt
                touch client_file.txt
        elif [[("$i"=="6")]];
        then
                touch baseDir/server_file.txt
        elif [[("$i"=="7")]];
        then 
                mkdir dir1
                echo "This is file 1." > baseDir/dir1/F1.txt
                echo "This is a file." > baseDir/A-F1.txt
                echo "This is file 1." > baseDir/F1.txt
                echo "This is file 2." > baseDir/F2.txt
                echo "Nothing here." > baseDir/F3.txt
                echo "Sample." > baseDir/Sample.txt
        elif [[("$i"=="8")]];
        then 
                mkdir dir1
                mkdir dir1/dir2
                echo "Some random text" > baseDir/F1.txt
                echo "Some other random text" > baseDir/F2.txt
                echo "This is file1." > baseDir/dir1/F1.txt
                echo "​random.guy@epfffl.ch" > baseDir/dir1/F1.txt
                echo "​random_guy@epfl.ch" > baseDir/dir1/dir2/F1.txt
                echo "​random.guy@epfl.ch" > baseDir/dir1/dir2/F2.txt
                echo "​randomguy@epfl.ch" > baseDir/dir1/F2.txt
        elif [[("$i"=="9")]];
        then
                for i in 1 ... 9999
                do
                        echo "This is file $i" > baseDir/File${i}.txt
                done
        fi
                 
        #bin/server > testcases/test${i}_server.out &
        bin/client 127.0.0.1 1337 testcases/test${i}.in testcases/test${i}_client.out
        echo
        echo "***** Test ${i} client output *****"
        cat testcases/test${i}_client.out
        echo
        echo "***** Test ${i} server output *****"
        cat testcases/test${i}_server.out
done        
        
        
        
        
