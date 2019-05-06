clear; close; clc;

%DAVIS-test dataset
DAVIS_list = {'blackswan','bmx-trees','breakdance','camel','car-roundabout',...
    'car-shadow','cows','dance-twirl','dog','drift-chicane',...
    'drift-straight','goat','horsejump-high','kite-surf',...
    'libby','motocross-jump','paragliding-launch','parkour','scooter-black','soapbox'};

FBMS_list = {'giraffes01','horses04','horses05','marple6','marple9'}; %for VSOP

SegTrackV2_TestList = {'bird_of_paradise','bmx','girl','monkey','parachute','soldier'};

%set Path
gtPath = '../Datasets/'
%Models = {'PDB','FGRNE','SIV','SP','TIMP','SAG','RWRV','GF','MB','MST','SGSP','SFLR','STBP','DLVSD','SCNN'};
Models={'SSAV'}

%VideoList = {'ViSal','FBMS','MCL','UVSD','VOS','DAVSOD','SegTrack-V2','DAVIS'}
VideoList = {'DAVSOD'}
NUM = length(VideoList);

for v = 1:NUM
    VideoName = VideoList{v};
      
    Thresholds = 1:-1/255:0;
    
    for m = 1:length(Models)
        modelName = Models{m}
        resVideoPath = ['../results/' modelName '/'];
        videoFiles = dir(gtPath);
        
        videoNUM = length(videoFiles)-2;
        
        video_Smeasure=zeros(1,videoNUM);
        video_adpFmeasure=zeros(1,videoNUM);
        
        video_Fmeasure = zeros(videoNUM,256);
        
        video_MAE=zeros(1,videoNUM);
        
        filePath = ['../evalResult/' modelName '/'];
        if ~exist(filePath,'dir')
            mkdir(filePath);
        end
        
        fileID = fopen([filePath VideoName '_result.txt'],'w');
        
        for videonum = 1:1
            %videofolder = videoFiles(videonum+2).name;
            videofolder = VideoName;
            
            seqPath = [gtPath videofolder '/'];
            seqFiles = dir(seqPath);
            
            seqNUM = length(seqFiles)-2;
            
            
            seq_Smeasure = zeros(1,seqNUM);
            seq_adpFmeasure = zeros(1,seqNUM);
            
            seq_Fmeasure = zeros(seqNUM,256);
            
            seq_MAE = zeros(1,seqNUM);
            
            for seqnum = 1: seqNUM
                
                seqfolder = seqFiles(seqnum+2).name;
                              
                gt_imgPath = [seqPath seqfolder '/ground-truth/'];
                [fileNUM, gt_imgFiles, fileExt] = calculateNumber(gt_imgPath); %index of stop frame
                
                resPath = [resVideoPath modelName '_' videofolder '/' seqfolder '/'];
                
                Smeasure = zeros(1,fileNUM-2);
                adpFmeasure = zeros(1,fileNUM-2);
                
                threshold_Fmeasure  = zeros(fileNUM-2,256);
                
                mae = zeros(1,fileNUM-2);
                
                tic;
                for i = 2:fileNUM-1 %skip the first and last gt file for some of the light-flow based method
                    
                    name = char(gt_imgFiles{i});
                    fprintf('Processing Seq:%s %d/%d Name: %s %d/%d\n',seqfolder,seqnum, seqNUM,name,i-1,fileNUM-2);
                    
                    %load gt
                    gt = imread([gt_imgPath name]);
                    if numel(size(gt))>2
                        gt = rgb2gray(gt);
                    end
                    if ~islogical(gt)
                        gt = gt(:,:,1) > 128;
                    end
                    
                    %load salency
                    sal  = imread([resPath name]);
                    
                    %check size
                    if size(sal, 1) ~= size(gt, 1) || size(sal, 2) ~= size(gt, 2)
                        sal = imresize(sal,size(gt));
                        imwrite(sal,[resPath name]);
                        fprintf('Error occurs in the path: %s!!!\n', [resPath name]);
                        %error('Saliency map and gt Image have different sizes!\n');
                        
                    end
                    
                    sal = im2double(sal(:,:,1));
                    
                    %normalize sal to [0, 1]
                    sal = reshape(mapminmax(sal(:)',0,1),size(sal));
                    
                    Smeasure(i-1) = StructureMeasure(sal,logical(gt));
                    
                    % Using the 2 times of average of sal map as the threshold.
                    threshold =  2* mean(sal(:)) ;
                    temp = Fmeasure_calu(sal,double(gt),size(gt),threshold);
                    adpFmeasure(i-1) = temp(3);
                    
                    
                    mae(i-1) = mean2(abs(double(logical(gt)) - sal));
                    
                    for t = 1:length(Thresholds)
                        threshold = Thresholds(t);
                        temp = Fmeasure_calu(sal,double(gt),size(gt),threshold);
                        threshold_Fmeasure(i-1,t) = temp(3);
                    end
                    
                end
                toc;
                
                
                seq_Smeasure(seqnum) = mean2(Smeasure);
                seq_adpFmeasure(seqnum) = mean2(adpFmeasure);
                
                seq_Fmeasure(seqnum,:) = mean(threshold_Fmeasure,1);
                seq_maxF = max(seq_Fmeasure(seqnum,:));
                seq_meanF = mean(seq_Fmeasure(seqnum,:));
                seq_MAE(seqnum) = mean2(mae);
                
                fprintf(fileID,'(%s Dataset)%s Seq Smeasure:%.3f;adpFmeasure:%.3f;maxF:%.3f;meanF:%.3f;MAE:%.3f\n',VideoName,seqfolder,seq_Smeasure(seqnum),seq_adpFmeasure(seqnum),seq_maxF,seq_meanF,seq_MAE(seqnum));
                
            end
            
            video_Smeasure(videonum) = mean2(seq_Smeasure);
            video_adpFmeasure(videonum) = mean2(seq_adpFmeasure);
            
            video_Fmeasure(videonum,:) = mean(seq_Fmeasure,1);
            maxF = max(video_Fmeasure(videonum,:));
            meanF = mean(video_Fmeasure(videonum,:));
            
            video_MAE(videonum) = mean2(seq_MAE);
         
            fprintf(fileID,'(%s Dataset) Smeasure:%.3f;adpFmeasure:%.3f;maxF:%.3f;meanF:%.3f;MAE:%.3f\n',VideoName,video_Smeasure(videonum),video_adpFmeasure(videonum),maxF,meanF,video_MAE(videonum));
            fprintf('(%s Dataset) Smeasure:%.3f;adpFmeasure:%.3f;maxF:%.3f;meanF:%.3f;MAE:%.3f\n',VideoName,video_Smeasure(videonum),video_adpFmeasure(videonum),maxF,meanF,video_MAE(videonum));
        end
        
        fclose(fileID);
        
    end
    
    
end




