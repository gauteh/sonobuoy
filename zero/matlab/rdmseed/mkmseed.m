function mkmseed(f,d,varargin)
%MKMSEED Write data in miniSEED file format.
%	MKMSEED(FILENAME,D,T0,FS) writes miniSEED file FILENAME from data vector D,
%	time origin T0 (in Matlab datenum-compatible format) and sampling rate FS
%	(in Hz). Encoding format will depend on D variable class (see below).
%
%	Network, Station, Channel and Location codes will be extracted from FILENAME
%	which must respect the format "NN.SSSSS.LC.CCC" where:
%		   NN = Network Code (2 characters max, see FDSN list)
%		SSSSS = Station identifier (5 char max)
%		   LC = Location Code (2 char max)
%		  CCC = Channel identifier (3 char max)
%	
%	Final filename will have appended string ".YYYY.DDD" corresponding to year
%	and ordinal day of origin time (from T0 value). Multiple files may be 
%	created if time span of data exceeds day limit.
%
%	MKMSEED(...,EF,RL) specifies encoding format EF and data record length RL
%	(in bytes). RL must be a power of 2 greater or equal to 256.
%
%	Data encoding format EF must be one of the following FDSN codes:
%		 1 = 16-bit integer (default for class 2-bit, 8-bit, int16)
%		 3 = 32-bit integer (default for class uint16, int32)
%		 4 = IEEE float32 (default for class single)
%		 5 = IEEE float64 (default for all other class)
%		10 = Steim-1 compression (D will be converted to int32)
%		11 = Steim-2 compression (D will be converted to int32)
%
%	MKMSEED(FILENAME,D) uses default value for T0 = now (present date and time),
%	and FS = 100 Hz.
%
%	File(s) will be coded big-endian, flags set to zero, blockette 1000, default
%	record length is 4096 bytes. Outputs have been tested with PQL II software
%	from IRIS PASSCAL (v2010.268).
%
%	See also RDMSEED function for miniSEED file reading.
%
%
%	Author: François Beauducel <beauducel@ipgp.fr>
%		Institut de Physique du Globe de Paris
%	Created: 2011-10-19
%	Updated: 2011-12-16
%
%	Acknowledgments:
%		Florent Brenguier, Julien Vergoz
%
%	References:
%		IRIS (2010), SEED Reference Manual: SEED Format Version 2.4, May 2010,
%		  IFDSN/IRIS/USGS, http://www.iris.edu
%		IRIS (2010), PQL II Quick Look trace viewing application, PASSCAL
%		  Instrument Center, http://www.passcal.nmt.edu/

%	Development history:
%	[2011-10-20]
%		- first version with integer and float encodings (no compression)
%		  and continuous time series (T0 + FS)
%
%	[2011-10-25]
%		- add Steim-1 and Steim-2 encoding formats
%
%	[2011-11-03]
%		- accepts column vector D
%
%	[2011-11-16]
%		- accepts void location code (thanks to Julien Vergoz)
%
%
%	Copyright (c) 2011, François Beauducel, covered by BSD License.
%	All rights reserved.
%
%	Redistribution and use in source and binary forms, with or without 
%	modification, are permitted provided that the following conditions are met:
%
%	   * Redistributions of source code must retain the above copyright 
%	     notice, this list of conditions and the following disclaimer.
%	   * Redistributions in binary form must reproduce the above copyright 
%	     notice, this list of conditions and the following disclaimer in 
%	     the documentation and/or other materials provided with the distribution
%	                           
%	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
%	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
%	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
%	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
%	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
%	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
%	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
%	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
%	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
%	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
%	POSSIBILITY OF SUCH DAMAGE.

error(nargchk(2,6,nargin))

% default input arguments
t0 = now;	% Time origin
fs = 100;	% Frequency sampling rate (Hz)
rl = 2^12;	% Record length

if ~ischar(f)
	error('FILENAME argument must be a string.');
else
	% check filename
	[fpath,fname,fext] = fileparts(f);
	cc = strread([fname,fext],'%s','delimiter','.');
	if length(cc) < 4
		error('FILENAME must be of the form: NN.SSSSS.LC.CCC');
	end
	% extract data channel identifiers
	X.NetworkCode = sprintf('%-2s',upper(cc{1}(1:min(length(cc{1}),2))));
	X.StationIdentifierCode = sprintf('%-5s',upper(cc{2}(1:min(length(cc{2}),5))));
	X.LocationIdentifier = sprintf('%-2s',cc{3}(1:min(length(cc{3}),3)));
	X.ChannelIdentifier = sprintf('%-3s',cc{4}(1:min(length(cc{4}),4)));
end

if ~isnumeric(d)
	error('Argument D must be numeric.');
else
	d = d(:);
end


if nargin > 2 & ~isempty(varargin{1})
	try
		t = datenum(varargin{1});
	catch
		error('Argument T or T0 must be valid date (see DATENUM).');
	end
	if numel(t) ~= 1 & numel(t) ~= numel(d)
		error('Argument T must be scalar or array of same size as D.');
	end
	t0 = t(1);
end

if nargin > 3 & ~isempty(varargin{2})
	fs = varargin{2};
	if ~isnumeric(fs) | numel(fs) > 1
		error('Argument FS must be a scalar.');
	end
elseif numel(t) > 1
	% guess sampling rate from vector T
	dt = diff(t);
	k = find(dt>0);
	if ~isempty(k)
		fs = 1/(86400*dt(k(1)));
	else
		warning('MKMSEED: cannot guess sampling rate from T. Use default value FS = 100 Hz');
	end
end

if nargin > 4 & ~isempty(varargin{3})
	ef = varargin{3};
	if ~isnumeric(ef) | numel(ef) > 1 | all(ef ~= [1,3,4,5,10,11])
		error('Argument EF not valid. Must be 1,3,4,5,10 or 11 (see help).');
	end
	% EF specified: converts D to correct class
	switch ef
	case 1
		d = int16(d);
	case {3,10,11}
		d = int32(d);
	case 4
		d = single(d);
	otherwise
		d = double(d);
	end
else
	% EF not specified: choose encoding consistent with data range
	switch class(d)
	case {'logical','char','uint8','int8','int16'}
		ef = 1;		% 16-bit
		d = int16(d);
	case {'uint16','int32'}
		ef = 3;		% 32-bit
		d = int32(d);
	case 'single'
		ef = 4;	% IEEE float32
	otherwise
		ef = 5;	% IEEE float64 (double)
		d = double(d);
	end

end

if nargin > 5
	rl = varargin{4};
	if ~isnumeric(rl) | numel(rl) > 1 | rl < 256 | mod(log(rl)/log(2),1) ~=0
		error('Argument RL must be a power of 2, scalar >= 256.');
	end
end

% reconstructs the filename base (without date)
fbase = fullfile(fpath,sprintf('%s.%s.%s.%s', ...
	deblank(X.NetworkCode),deblank(X.StationIdentifierCode), ...
	deblank(X.LocationIdentifier),deblank(X.ChannelIdentifier)));

X.SequenceNumber = 1;
day0 = 0;

% number of bytes per data
dl = length(typecast(d(1),'int8'));

% number of samples in a data block
switch ef
case {1,3,4,5}
	nbs = (rl - 64)/dl;
case 10
	nbs = ((rl/64-1)*15 - 1)*4;	% Steim-1 maximum compression is four 8-bit per longword
case 11
	nbs = ((rl/64-1)*15 - 1)*7;	% Steim-2 maximum compression is seven 4-bit per longword
end

n = 1;
while n < length(d)

	% start time of block
	X.StartTime = t0 + (n - 1)/(86400*fs);
	tv = datevec(X.StartTime);
	day = floor(datenum(tv)) - datenum(tv(1),1,0);

	% filename with year and day
	fn = sprintf('%s.%04d.%03d',fbase,tv(1),day);

	% open a new file
	if day ~= day0
		if day0 ~= 0
			fclose(fid);
		end
		fid = fopen(fn,'wb','ieee-be');
		if fid == -1
			error('Cannot create file "%s".',fn);
		end
		fprintf('MKMSEED: file "%s" created.\n',fn);
		day0 = day;
	end

	% index of data in the block
	k = n:min(n + nbs - 1,length(d));
	X.data = d(k);
	nn = write_data_record(fid,X,fs,ef,rl);
	n = k(nn) + 1;
	X.SequenceNumber = X.SequenceNumber + 1;

end

fclose(fid);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function n = write_data_record(fid,X,fs,ef,rl)
% N=write_data_record(FID,X,FS,EF,RL) writes a single data record block using
% FID file ID, structure X containing header and data information,
% sample rate FS, encoding format EF, record length RL.
% Returns the number of data N effectively written.


% --- prepare the data
switch ef
case {1,3,4,5} % int16, int32, float32, float64: just writes the data flow!
	data = X.data;
	dl = length(typecast(X.data(1),'int8'));
	n = length(X.data);
	nrem = rl - n*dl - 64;
case {10,11} % Steim-1/2: a bit more complicated...
	nb64 = rl/64-1;
	nibble = zeros(15,nb64);
	steim = zeros(15,nb64,'uint32');
	switch ef
	case 10 % Steim-1
		bnib = [8,16,32];	% number of bit
		nnib = [4,2,1];		% number of differences
		cnib = [1,2,3];		% Cnib value
	case 11 % Steim-2
		bnib = [4,5,6,8,10,15,30];	% number of bit
		nnib = [7,6,5,4,3,2,1];		% number of differences
		cnib = [3,3,3,1,2,2,2];		% Cnib value
		dnib = [2,1,0,-1,3,2,1];	% Dnib value
	end
	steim(1) = typecast(X.data(1),'uint32'); % X0
	dd = [0;diff(double(X.data))]; % computes the differences (D0=0)
	ldd = length(dd);
	n = 0;
	for k = 3:numel(steim) % loop on each 32-bit word in the block
		for kk = 1:length(bnib) % will try each compression scheme from max to min
			if (n + nnib(kk)) <= ldd % first condition is to not exceed data length!
				ddd = dd(n + (1:nnib(kk)));
				sbn = 2^(bnib(kk)-1);
				if length(find(ddd>=-sbn & ddd<=(sbn-1)))==nnib(kk)
					nibble(k) = cnib(kk);
					steim(k) = bitjoin(double(bitsignset(ddd,bnib(kk))),bnib(kk));
					% 2-bit dnib for Steim-2 other than four 8-bit
					if ef==11 & dnib(kk) >= 0
						steim(k) = bitset(steim(k),31,bitget(uint8(dnib(kk)),1));
						steim(k) = bitset(steim(k),32,bitget(uint8(dnib(kk)),2));
					end
					n = n + nnib(kk);
					break;
				end
			end
		end
        	if n >= length(dd)
        		break;
		end
	end
	steim(2) = typecast(X.data(n),'uint32'); % Xn (for inverse integration check)
	% fill-in data block with nibbles and 32-bit words
	data = zeros(16,nb64,'uint32');
	for k = 1:nb64
		data(1,k) = bitjoin([0;nibble(:,k)],2);
		data(2:16,k) = steim(:,k);
	end
	nrem = 0;
end


% --- write Data Header (48 bytes)

fwrite(fid,sprintf('%06d',X.SequenceNumber),'char');
fwrite(fid,'D','char');	% data header/quality indicator
fwrite(fid,' ','char');	% reserved byte
fwrite(fid,X.StationIdentifierCode,'char');
fwrite(fid,X.LocationIdentifier,'char');
fwrite(fid,X.ChannelIdentifier,'char');
fwrite(fid,X.NetworkCode,'char');

% start time
tv = datevec(X.StartTime);
fwrite(fid,tv(1),'uint16');
fwrite(fid,floor(datenum(tv))-datenum(tv(1),1,0),'uint16');
fwrite(fid,tv(4),'uint8');
fwrite(fid,tv(5),'uint8');
fwrite(fid,floor(tv(6)),'uint8');
fwrite(fid,0,'uint8');
fwrite(fid,1e4*mod(tv(6),1),'uint16');

% number of samples
fwrite(fid,n,'uint16');

% sample rate: calculate the maximum possible multiplier to preserve precision
if fs >= 1
	p2 = 2^(14-ceil(log(fs)/log(2)));
	fwrite(fid,fs*p2,'int16');	% Sample rate factor (Hz)
	fwrite(fid,-p2,'int16');	% Sample rate multiplier (division)
else
	p2 = 2^(14-ceil(log(1/fs)/log(2)));
	fwrite(fid,-p2/fs,'int16');	% Sample rate factor (s)
	fwrite(fid,p2,'int16');	% Sample rate multiplier
end

fwrite(fid,[0,0,0],'uint8');	% Activity, I/O, Data Quality flags
fwrite(fid,1,'uint8');		% Number Blockettes Follow
fwrite(fid,0,'float32');	% Time Correction
fwrite(fid,64,'uint16');	% Offset Begin Data
fwrite(fid,48,'uint16');	% Offset First Blockette

% blockette 1000 (8 bytes)
fwrite(fid,1000,'uint16');	% Blockette type
fwrite(fid,0,'uint16');		% Offset Next Blockette
fwrite(fid,ef,'uint8');		% Encoding Format
fwrite(fid,1,'uint8');		% Word Order (big-endian)
fwrite(fid,round(log(rl)/log(2)),'uint8');	% Data Record Length
fwrite(fid,0,'uint8');		% Reserved

% complete header to reach standard 64 bytes length
fwrite(fid,zeros(8,1),'int8');

% --- write the data
fwrite(fid,data(:),class(data));

% leading null to respect Data Record Length
if nrem > 0
	fwrite(fid,zeros(nrem,1),'int8');
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function d = bitjoin(x,n,b)
% bitjoin(X,N,B) joins the N-bit array X into one unsigned B-bit number
%	X must be unsigned integer class
%	B must be >= N*length(X)
% (This is the reverse function of bitsplit in RDMSEED)

d = uint32(sum(flipud(x(:)).*2.^(0:n:(length(x)-1)*n)'));


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function d = bitsignset(x,n)
% bitsignset(X,N) returns unsigned N-bit coded from signed double X.
% Result is coded as uint32 class.
% (This is the reverse function of bitsign in RDMSEED)

d = uint32(x + (x<0)*2^n);

