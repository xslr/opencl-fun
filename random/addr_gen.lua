#!/usr/bin/lua

--[[
-- Code to test indices of sample data used to compute FFT during each FFT length
--]]

length = 1024
radix  = 2
thread_id = 2

print("FFT Address Generator Test. " .. length .. " points, radix " .. radix .. " version")
print("thread id is " .. thread_id)

fft_size = 2

while fft_size <= length do
	print("FFT size " .. fft_size)

	house = math.floor(thread_id * radix / fft_size)
	print("\thouse is " .. house)

	house_base = house * fft_size
	print("\thouse_base is " .. house_base)

	house_offset = thread_id % (fft_size/2)
	print("\thouse offset is " .. house_offset)

	index_of_interest = house_base + house_offset
	print("\tindex of interest is " .. index_of_interest)
	
	fft_size = fft_size * radix
end
