clc
clear all
close all
 
myReadChannelID = 1908356;
readAPIKey = 'CVABFRXNOBUTIE38';
 
[tempData,myTimeStamps] = thingSpeakRead(myReadChannelID, 'ReadKey',readAPIKey, ...
    'numMinutes',120,'Fields',1);
 
 
%tempData1 = [tempData(8:end)];
%myTimeStamps1 = [myTimeStamps(8:end)];
 
%Converting a time array into a int array to create transfer function
x=1:length(myTimeStamps);
 
%DATA PLOT
plot(x,tempData,'ob','linewidth',1);
grid on;
hold on;
 
%Linear Transfer Function -> Less Computational Operation
coeff = polyfit(x,tempData,1);
y1 = polyval(coeff,x);
 
%Higher Order Transfer Function -> Higher Computational Operation, better fit
coeff = polyfit(x,tempData,3)
y2 = polyval(coeff,x);
 
plot(x,y1,'g','lineWidth',2); 
hold on;
plot(x,y2,'r','lineWidth',2); 
xlabel('Time (minutes)','FontName','Curlz MT','FontSize',12.5)
ylabel('M.y','FontName','Curlz MT','FontSize',12.5)
legend('Data','Linear Model','Higher Order Model')
