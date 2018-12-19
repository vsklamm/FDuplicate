#!/bin/sh

cancel_message="Generating was cancelled"
echo -n "Tests require about 5 GB of disk space. Continue? (y/n) "
read item
case "$item" in y|Y) ;; n|N) echo $cancel_message; exit 0 ;; *) echo $cancel_message; exit 0 ;;  esac

if [ -d "fduplicate_tests" ]; then
  	echo -n "Directory 'fduplicate_tests' is already exists.\
	We do not advise you to generate in the same folder. Continue? (y/n)"
	read item
	case "$item" in y|Y) ;; n|N) echo $cancel_message; exit 0 ;; *) echo $cancel_message; exit 0 ;; esac
fi

. ./test_functions.sh

dir="$(pwd)"
goto_dir fduplicate_tests

echo '== RUNTIME =='

goto_dir runtime
echo 'TEST 1. (3000 x 4Kb, 0 dupes)'
goto_dir test1
for i in $(seq 1 1 3000); do 
	(dd if=/dev/urandom of=$i bs=4K count=1) > /dev/null 2>&1;
# generate_files_group 3000 0 1 4K
done
cd ..

echo 'TEST 2. (3000 x 1M, ~60% dupes)'
goto_dir test2
generate_files_group 3000 60 1 1M
cd ..

echo 'TEST 3. (2 x 1Gb, 2 dupes)'	
goto_dir test3
generate_files_group 2 100 100 1G
cd ..

echo 'TEST 4. (for my bug)'	
goto_dir test4
generate_files_group 24 100 25 1M
cd ..

cd ..

echo '== SYMLINKS =='

goto_dir symlinks
echo 'TEST 3. (3 equal files <-> 2 symlinks, 1 broken symlink)'
goto_dir test1 
# outer="$(pwd)"
generate_files_group 3 100 100 100K
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


