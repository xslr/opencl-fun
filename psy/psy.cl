__kernel void psy_do( __global float samples,
					  __global ushort blk_type,
					  __global ushort *grouping,
					  __global float thr)
{
	window();

	threshold();
}

void window()
{
	// decide current block window length

	// read length of next block

	// update current block type !! handling of last block
}

void threshold()
{
	// calculate energy in sfb

	// convert energy to threshold

	// threshold in quiet

	// pre-echo control
}
