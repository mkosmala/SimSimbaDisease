library(abc)

# read first round results (even distribution over parameter space)
dataFullStats = read.csv(file="round1-scored_not-extinct.csv",head=TRUE)

# do the cross-validation

# extract parameters and renumber rows
dFSParam=dataFullStats[,3:7]
row.names(dFSParam)=1:nrow(dFSParam)

# extract summary statistics
dFSSumStat=data.frame(DeltaPopNorth=dataFullStats[,8]-dataFullStats[,11],
			    DeltaPopCentral=dataFullStats[,9]-dataFullStats[,12],
			    DeltaPopSouth=dataFullStats[,10]-dataFullStats[,13],
			    NorthBTB1999=dataFullStats[,14],
			    CentralBTB1999=dataFullStats[,15],
			    SouthBTB1999=dataFullStats[,16])

# save tables
#write.csv(dFSParam,"round1_params.csv",sep=",")
#write.csv(dFSSumStat,"round1_sum_stats.csv",sep=",")

# cross-validation
val = cv4abc(param=dFSParam,sumstat=dFSSumStat,
		 nval=100,tols=0.1,method="neuralnet")

# look at validation
plot(val)

# extract values and estimates
xvals = val$true
row.names(xvals)=1:nrow(xvals)
yvals = as.data.frame(val$estim$tol0.1)

# save values and estimates
#write.csv(xvals,"true_values.csv")
#write.csv(yvals,"estimated_values.csv")

# calculate correlations
cor(xvals$L,yvals$L)
cor(xvals$B,yvals$B)
cor(xvals$O,yvals$O)
cor(xvals$E,yvals$E)
cor(xvals$I,yvals$I)

# plot correlations
tiff("cross-validation.tiff",width=1300,height=300)
par(mfrow=c(1,5),mar=c(5,5,4,1.5),cex.lab=2,cex.main=3)

plot(xvals$L,yvals$L,xlim=c(0,1),ylim=c(0,1),pch=3,col="black",
	xlab="True value",ylab="Estimated value")
title("L")
abline(0,1)

plot(xvals$B,yvals$B,xlim=c(0,1),ylim=c(0,1),pch=3,col="black",
	xlab="True value",ylab="Estimated value")
title("B")
abline(0,1)

plot(xvals$O,yvals$O,xlim=c(0,1),ylim=c(0,1),pch=3,col="black",
	xlab="True value",ylab="Estimated value")
title("O")
abline(0,1)

plot(xvals$E,yvals$E,xlim=c(0,1),ylim=c(0,1),pch=3,col="black",
	xlab="True value",ylab="Estimated value")
title("E")
abline(0,1)

plot(xvals$I,yvals$I,xlim=c(0,1),ylim=c(0,1),pch=3,col="black",
	xlab="True value",ylab="Estimated value")
title("I")
abline(0,1)

dev.off()




