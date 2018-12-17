#!/bin/sh

echo -n "Tests require about 5 GB of disk space. Continue? (y/n) "
read item
case "$item" in
    y|Y) 
	;;
    n|N) 
	exit 0
	;;
    *) 
	exit 0
	;;
esac

. ./test_functions.sh

dir="$(pwd)"
goto_dir fduplicate_tests

echo '== RUNTIME =='

goto_dir speed
echo 'TEST 1. (3000 x 4Kb, 0 dupes)'
goto_dir test1
for i in $(seq 0 1 3000); do 
	(dd if=/dev/urandom of=$i bs=4096 count=1) > /dev/null 2>&1; 
# generate_files_group 3000 0 1 4096
done
cd ..

echo 'TEST 2. (3000 x 1M, ~60% dupes)'
goto_dir test2
generate_files_group 3000 60 1 1M
cd ..

echo 'TEST 3. (2 x 1Gb, 2 dupes)'	
goto_dir test2
generate_files_group 2 100 50 1Gb
cd ..

cd ..

echo '== SYMLINKS =='

goto_dir symlinks
echo 'TEST 3. (2 equal files <-> 2 symlinks, 1 broken symlink)'
goto_dir test1 
generate_files_group 2 100 50 100K
cd ..

echo 'TEST 4. (loop, 2 equal files)'
goto_dir test2
outer="$(pwd)"
inner=nested
mkdir -p $inner
ln -s $outer $outer/$inner/nested_link_to_outer 
cd ..

cd ..

goto_dir rights
echo '== RIGHTS =='
echo 'TEST 5. ()'

echo 'TEST 6. ()'
cd ..


