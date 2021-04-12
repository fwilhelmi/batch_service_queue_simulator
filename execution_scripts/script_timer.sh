# define execution parameters
SIM_TIME=1000000
LOGS_ENABLED=0
QUEUE_SIZE=10
BATCH_SIZE_ARRAY=( 1 2 3 4 5 6 7 8 9 10 )
LAMBDA_ARRAY=( 2.5 5 7.5 10 12.5 15 )
MU=15
TIMEOUT_MINING=( 0.5 2 5 )
SEED=12345

# compile code
cd ..
cd main
./build_local
echo 'EXECUTING MULTIPLE SIMULATIONS... '
cd ..
# remove old script output file and node logs
rm output/*

# execute files
cd main
pwd
for timer in "${TIMEOUT_MINING[@]}"
do 
for lambda in "${LAMBDA_ARRAY[@]}"
do 
for batch_size in "${BATCH_SIZE_ARRAY[@]}"
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING timer = ${timer} lambda = ${lambda}, block size = ${batch_size}"
	./queue_main $SIM_TIME $LOGS_ENABLED $QUEUE_SIZE ${batch_size} ${lambda} $MU ${timer} $SEED ../output/script_output_t${timer}_l${lambda}_s${batch_size}.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
done
done
echo ""
echo 'SCRIPT FINISHED: OUTPUT FILE SAVED IN /output/script_output.txt'
echo ""
echo ""
