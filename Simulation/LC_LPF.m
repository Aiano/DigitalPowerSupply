% 假设电感值和电容值
L = 1e-3; % 电感值（单位：亨利）
C = 1e-6; % 电容值（单位：法拉）

% 计算截止频率
wc = 1 / sqrt(L * C);

% 频率范围
w = linspace(10, 100000, 100);

% LC低通滤波器的传递函数
H = 1 ./ (1 -  w.^2 * L * C);

% 绘制幅频特性曲线
figure;
semilogx(w, 20 * log10(abs(H)));
xlabel('角频率');
ylabel('|H(j\omega)| (dB)');
title('LC低通滤波器幅频特性曲线');
grid on;