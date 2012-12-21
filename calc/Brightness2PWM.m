% real exponential characteristic
b=0:65535;
x=exp(b*0.0001);
y=(x-1.0) * (65535.0 / (exp(6.5535)-1.0));
plot(b,y);
min(y)
max(y)

% approximation
bits=5;
steps=2^(16-bits);
i=[0:steps:65535,65535];
yi=round(y(i+1));
yiif=floor(interp1(i,yi,b));
errf=(y-yiif)./y;
sum(abs(errf(isfinite(errf)))) / sum(isfinite(errf))

hold on;
plot(b,yiif,'r');

yiir=round(interp1(i,yi,b));
errr=(y-yiir)./y;
sum(abs(errr(isfinite(errr)))) / sum(isfinite(errr))

plot(b,yiir,'m');

fid=1;  % stdout
fprintf(fid,'#define BRIGHTNESS2PWM_VALUES_BITS    %d\n',bits);
fprintf(fid,'#define BRIGHTNESS2PWM_VALUES_SHIFT   (16 - BRIGHTNESS2PWM_VALUES_BITS)\n');
fprintf(fid,'#define BRIGHTNESS2PWM_VALUES_MASK    ((1 << BRIGHTNESS2PWM_VALUES_SHIFT)-1)\n');
fprintf(fid,'#define BRIGHTNESS2PWM_VALUES_COUNT   ((1 << BRIGHTNESS2PWM_VALUES_BITS) + 1)\n');
fprintf(fid,'const uint16_t Brightness2PWMValues[BRIGHTNESS2PWM_VALUES_COUNT] = {\n');
fprintf(fid,' ');
for n = 1:(length(yi)-1)
  fprintf(fid,' %5d,',yi(n));
  if mod(n,16) == 0
    fprintf(fid,'\n ');
  end
end
fprintf(fid,' %5d\n',yi(end));
fprintf(fid,'};\n');
