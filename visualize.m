i = 1;
for i = 1:10
offset = 17;

depth = sprintf('%04d.png', i);
object = sprintf('%04d.png', i+offset);
lit = sprintf('%04d.png', i+offset*2);
unlit = sprintf('%04d.png', i+offset*3);

subplot(221); imshow(imread(depth));
subplot(222); imshow(imread(object));
subplot(223); imshow(imread(lit));
subplot(224); imshow(imread(unlit));
pause
end