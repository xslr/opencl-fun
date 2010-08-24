#!/usr/bin/lua

--[[
-- Twiddle factor generation algorithm test.
--]]

thread_id = 0
length = 16

radix = 2

while thread_id < (length / 2) do
	print("thread id is " .. thread_id)
	fft_size = 1
	while fft_size < length do
		fft_size = fft_size * radix

		stride = length / fft_size
		print("\tstride is " .. stride)

		twiddle_exp = (thread_id % (fft_size/2)) * stride
		print("twiddle exponent is " .. twiddle_exp)
	end

	thread_id = thread_id + 1
end
