function [NUM,file,fileExt] = calculateNumber(imgPath)
    imgExt = {'*.bmp', '*.jpg', '*.png'};
    k=1;
    d1 = dir([imgPath char(imgExt(k))]);
    file = {d1(~[d1.isdir]).name};
    if isempty(file)
        k = k + 1;
        d1 = dir([imgPath char(imgExt(k))]);
        file = {d1(~[d1.isdir]).name};
        if isempty(file)
            k = k + 1;
            d1 = dir([imgPath char(imgExt(k))]);
            file = {d1(~[d1.isdir]).name};
            NUM = length(file);
        else
            NUM = length(file);
        end
    else
        NUM = length(file);
    end
    fileExt = char(imgExt(k));
end