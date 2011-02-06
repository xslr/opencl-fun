
function y = mdct4(x)
% MDCT4 Calculates the Modified Discrete Cosine Transform
%   y = mdct4(x)
%
%   Use either a Sine or a Kaiser-Bessel Derived window (KBDWin)with 
%   50% overlap for perfect TDAC reconstruction.
%   Remember that MDCT coefs are symmetric: y(k)=-y(N-k-1) so the full
%   matrix (N) of coefs is: yf = [y;-flipud(y)];
%
%   x: input signal (can be either a column or frame per column)
%      length of x must be a integer multiple of 4 (each frame)     
%   y: MDCT of x (coefs are divided by sqrt(N))
%
%   Vectorize ! ! !

% ------- mdct4.m ------------------------------------------
% Marios Athineos, marios@ee.columbia.edu
% http://www.ee.columbia.edu/~marios/
% Copyright (c) 2002 by Columbia University.
% All rights reserved.
% ----------------------------------------------------------

[flen,fnum] = size(x);
% Make column if it's a single row
if (flen==1)
    x = x(:);
    flen = fnum;
    fnum = 1;
end
% Make sure length is multiple of 4
if (rem(flen,4)~=0)
    error('MDCT4 defined for lengths multiple of four.');
end

% We need these for furmulas below
N     = flen; % Length of window
M     = N/2;  % Number of coefficients
N4    = N/4;  % Simplify the way eqs look
sqrtN = sqrt(N);

% Preallocate rotation matrix
% It would be nice to be able to do it in-place but we cannot
% cause of the prerotation.
rot = zeros(flen,fnum);

% Shift
t = (0:(N4-1)).';
rot(t+1,:) = -x(t+3*N4+1,:);
t = (N4:(N-1)).';
rot(t+1,:) =  x(t-N4+1,:);
clear x;

csvwrite('rot-ml.csv', transpose(rot))

% We need this twice so keep it around
t = (0:(N4-1)).';
%w = diag(sparse(exp(-j*2*pi*(t+1/8)/N)));

w2 = exp(-j*2*pi*(t+1/8)/N);
%plot(imag(w2))

% Pre-twiddle
t = (0:(N4-1)).';
c =   (rot(2*t+1,:)-rot(N-1-2*t+1,:)) -j*(rot(M+2*t+1,:)-rot(M-1-2*t+1,:));

csvwrite('c-ml-r.csv', transpose(real(c)))
csvwrite('c-ml-i.csv', transpose(imag(c)))
csvwrite('w-ml-r.csv', transpose(real(w2)))
csvwrite('w-ml-i.csv', transpose(imag(w2)))

%plot(imag(c))

% This is a really cool Matlab trick ;)
%c = 0.5*w*c;
c = 0.5*c.*w2;
clear rot;

%subplot(211)
%plot(real(c), 'r')
%subplot(212)
%plot(imag(c), 'b')

%plot(imag(c))
%plot(real(c))

% FFT for N/4 points only !!!
c = fft(c,N4);

% Post-twiddle
c = (2/sqrtN)*w2.*c;

% Sort
t = (0:(N4-1)).';
y(2*t+1,:)     =  real(c(t+1,:));
y(M-1-2*t+1,:) = -imag(c(t+1,:));

endfunction
