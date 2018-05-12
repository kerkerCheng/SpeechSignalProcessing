#!/bin/bash

source set_htk_path.sh

config=lib/config.cfg
data_list=scripts/training.scp

mmf_dir=hmm
macro=$mmf_dir/macros
model=$mmf_dir/models

label=labels/Clean08TR.mlf
model_list=lib/models.lst

#################################################
# re-adjust mean, var
echo "step 01 [HErest]: adjust mean, var..."
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30;
do
	echo "iteration $i"
	HERest -C $config -I $label \
		-t 250.0 150.0 1000.0 -S $data_list \
		-H $macro -H $model -M $mmf_dir $model_list
done

#################################################
# add short pause model, change model_list and label file
echo "step 02 [HHEd]: add \"sp\" model"

if [ ! -e bin/spmodel_gen ]; then
	cd bin/; make ; cd ..;
fi
bin/spmodel_gen $model $model
label=labels/Clean08TR_sp.mlf
model_list=lib/models_sp.lst
HHEd -T 2 -H $macro -H $model -M $mmf_dir lib/sil1.hed $model_list

# re-adjust mean, var
echo "step 03 [HErest]: adjust mean, var..."
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30;
do
	echo "iteration $i"
	HERest -C $config -I $label \
		-t 250.0 150.0 1000.0 -S $data_list \
		-H $macro -H $model -M $mmf_dir $model_list
done


#################################################
# increase mixture
echo "step 04 [HHEd]: split gaussian mixture..."
HHEd -T 2 -H $macro -H $model -M $mmf_dir lib/mixAdd2_10.hed $model_list

# re-adjust mean, var
echo "step 05 [HERest]: adjust mean, var..."
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60;
do
	HERest -C $config -I $label \
		-t 250.0 150.0 1000.0 -S $data_list \
		-H $macro -H $model -M $mmf_dir $model_list
done

