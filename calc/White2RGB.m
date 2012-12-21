% # download Mitchell Charity's Blackbody color datafile
% wget http://www.vendian.org/mncharity/dir3/blackbody/UnstableURLs/bbr_color.txt
%
% # filter 10 degree CMFs and only use RGB Data
% grep 'K  10deg' bbr_color.txt | awk '{print $1 " " $7 " " $8 " " $9 " " $10 " " $11 " " $12;}' > bbr_color_10deg_rgb.txt
%
% load data
bbr=load('bbr_color_10deg_rgb.txt','ascii');
% separate into variables
T = bbr(:,1);
r = bbr(:,2).^(1/2.2);   % reverse gamma-correction
g = bbr(:,3).^(1/2.2);
b = bbr(:,4).^(1/2.2);
R = bbr(:,5);
G = bbr(:,6);
B = bbr(:,7);
% check linearity
hold on
plot(r,R,'r');
plot(g,G,'g');
plot(b,B,'b');
plot([0 1],[0 255],'k');

% approximation
bits=6;
steps=2^(16-bits);
start=430;  % it is important to meet 6600K (or better 6574K)
i=start:steps:40000;

rr=round(interp1(T,r,i)*65535);
gr=round(interp1(T,g,i)*65535);
br=round(interp1(T,b,i)*65535);

rr(1) = rr(2);
gr(1) = round(gr(2) - (gr(2)-g(1)*65535)/(i(2)-T(1))*steps);
br(1) = 0;

figure
hold on
plot(T,r,'r');
plot(T,g,'g');
plot(T,b,'b');
plot(i,rr/65535,'-m.','MarkerSize',30);
plot(i,gr/65535,'-c.','MarkerSize',30);
plot(i,br/65535,'-k.','MarkerSize',30);


fid=1;  % stdout
fprintf(fid,'#define WHITE2RGB_VALUES_BITS    %d\n',bits);
fprintf(fid,'#define WHITE2RGB_VALUES_START   %d\n',start);
fprintf(fid,'#define WHITE2RGB_VALUES_SHIFT   (16 - WHITE2RGB_VALUES_BITS)\n');
fprintf(fid,'#define WHITE2RGB_VALUES_MASK    ((1 << WHITE2RGB_VALUES_SHIFT)-1)\n');
fprintf(fid,'#define WHITE2RGB_VALUES_COUNT   %d\n',length(i));
fprintf(fid,'const uint16_t White2RGBRed[WHITE2RGB_VALUES_COUNT] = {\n');
fprintf(fid,' ');
for n = 1:(length(i)-1)
  fprintf(fid,' %5d,',rr(n));
  if mod(n,16) == 0
    fprintf(fid,'\n ');
  end
end
fprintf(fid,' %5d\n',rr(end));
fprintf(fid,'};\n');
fprintf(fid,'const uint16_t White2RGBGreen[WHITE2RGB_VALUES_COUNT] = {\n');
fprintf(fid,' ');
for n = 1:(length(i)-1)
  fprintf(fid,' %5d,',gr(n));
  if mod(n,16) == 0
    fprintf(fid,'\n ');
  end
end
fprintf(fid,' %5d\n',gr(end));
fprintf(fid,'};\n');
fprintf(fid,'const uint16_t White2RGBBlue[WHITE2RGB_VALUES_COUNT] = {\n');
fprintf(fid,' ');
for n = 1:(length(i)-1)
  fprintf(fid,' %5d,',br(n));
  if mod(n,16) == 0
    fprintf(fid,'\n ');
  end
end
fprintf(fid,' %5d\n',br(end));
fprintf(fid,'};\n');
