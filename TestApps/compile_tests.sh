#!/bin/sh



echo "compile_tests.sh: rebuild tests"

##rm memhog_rand_mt
##g++ -g memhog_mt_wrapper.cpp memhog_mt_randaccess.cpp ../src/libzsim/zsim_nic_defines.cpp -std=c++11 -o memhog_rand_mt -O3 -pthread
##
##rm memhog_ser_mt
##g++ -g memhog_mt_wrapper.cpp memhog_mt_seraccess.cpp ../src/libzsim/zsim_nic_defines.cpp -std=c++11 -o memhog_ser_mt -O3 -pthread
##
##rm memhog_ser_mt_addrand
##g++ -g memhog_mt_wrapper.cpp memhog_mt_seraccess_addrand.cpp ../src/libzsim/zsim_nic_defines.cpp -std=c++11 -o memhog_ser_mt_addrand -O3 -pthread
##
##rm memhog_rand_mix_mt
##g++ -g memhog_mt_wrapper.cpp memhog_mt_randaccess_mix.cpp ../src/libzsim/zsim_nic_defines.cpp -std=c++11 -o memhog_rand_mix_mt -O3 -pthread
##
##rm pointer_chaser
##g++ -g pointer_chaser.cpp  -std=c++11 -o pointer_chaser -O3
##
##rm ser_access_st
##g++ -g ser_access_st.cpp  -std=c++11 -o ser_access_st -O3



rm memhog_mt
#g++ -g memhog_mt_wrapper.cpp memhog_mt.cpp -std=c++11 -o memhog_mt -O3 -pthread
g++ -g memhog_mt_wrapper.cpp memhog_mt.cpp  -o memhog_mt -O3 -pthread

rm simple_array
g++ simple_array.cpp -o simple_array


