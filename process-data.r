# load the car and pls package

library(car)
library(pls)

# read the original data -- don't change this file!
filedata = read.csv(file="TrimmedOutput/R1IO.csv",head=TRUE)

# transform the data to be strictly positive
allsimdata = filedata[,6:11]
datamins = c(0,0,0,-750,-1000,-500)
datalen = dim(allsimdata)[1]
submatrix = matrix(rep(datamins,each=datalen),nrow=datalen)
knp = allsimdata - submatrix + 1

# do everything on just the first 1000 data points
knp1 = knp[1:1000,]

# box-cox transform all the summary statistics
tcoef = powerTransform(knp1)
knp2 = bcPower(knp1,coef(tcoef))

# write the results to a file
names(knp2) = names(knp1)
#write.table(knp2,file="boxcoxed.csv",quote=FALSE,sep=",",row.names=FALSE)

# run a PLS on the transformed data
knp3 = cbind(filedata[1:1000,1:5],knp2)
plsrtrans = plsr(cbind(L,B,O,E,I)~cbind(NorthBTB,CentralBTB,SouthBTB,NorthPop,CentralPop,SouthPop),data=knp3,scale=TRUE,validation="LOO")

#plot(RMSEP(plsrtrans))

numcomp = 3

# now transform the observed data
obs = data.frame(t(c(0,18,50,0,0,0)))
obs1 = obs - datamins + 1
obs2 = bcPower(obs1,coef(tcoef))
names(obs2) = names(knp)

# and then PLS the observed data
obs3 = predict(plsrtrans,comps=1:numcomp,newdata=obs2,type="scores")


#---------------------------
# Change #-round file here!!!
#---------------------------

# load new data and get it in the right format
newround = read.csv(file="TrimmedOutput/R5IO.csv",head=TRUE)

# transform the data to be strictly positive
nrstats = newround[,6:11]
names(nrstats)=names(knp)
tempn = cbind(newround[,1:5],nrstats)
newround = tempn

# datamins defined based on first time around
datalen = dim(nrstats)[1]
submatrix = matrix(rep(datamins,each=datalen),nrow=datalen)
nr1 = nrstats - submatrix + 1

# transform all the simulated data -- first Box-Cox
nr2 = bcPower(nr1,coef(tcoef))
names(nr2) = names(knp)

# and then PLS
nr3 = predict(plsrtrans,comps=1:numcomp,newdata=nr2,type="scores")

# now do the difference
obsrep = matrix(rep(obs3,each=datalen),nrow=datalen)
nr4 = nr3 - obsrep

# square each entry
nr5 = nr4*nr4

# sum across the rows
nr6 = apply(nr5,1,sum)

# write the results to a file
newround$Score = nr6
write.table(newround,file="scored.csv",quote=FALSE,sep=",",row.names=FALSE)

# Now sort them and keep the top 1000
sortdata = newround[order(newround$Score),]
topdata = sortdata[1:1000,]

# write the results to a file
write.table(topdata,file="top.csv",quote=FALSE,sep=",",row.names=FALSE)

# ---
# Weight them and normalize the weights
# ---
# for first round
#partweights = rep(1/1000,1000)

# for subsequent rounds, calculate the weight
# calculate the sum
# get the particles from the previous round (in the populations directory)
# and the kernel size used to create the current particles
# -------
# RESET PAST ROUND INFO HERE
# -------
lastround = read.csv(file="Populations/population3.csv",head=TRUE)
kernelsize = read.csv(file="Kernels/kernelsd4.csv",head=TRUE)

# unnormalized weights
unnorm = rep(0,1000)

# these only need to be constructed once
jrows = as.matrix(lastround[,1:5])
kernrep = matrix(as.numeric(rep(kernelsize,each=1000)),nrow=1000)   

# for each new particle
for (i in 1:1000) {

   # summation
   total = 0
 
   # see if any component is outside range between old particle and new particle
   rowrep = matrix(as.numeric(rep(topdata[i,1:5],each=1000)),nrow=1000)
   outside = kernrep - abs(rowrep-jrows) < 0
   keep = !(outside[,1]|outside[,2]|outside[,3]|outside[,4]|outside[,5])

   # calculate the sum and its inverse
   unnorm[i] = 1/sum(as.numeric(keep) * lastround[,6])
}

# normalize
partweights = unnorm/sum(unnorm)

# 
weightpart = cbind(topdata[,1:5],partweights)
names(weightpart)[6] = "Weight"

# -------
# RESET ROUND INFO HERE
# -------
# write to results to a file
write.table(weightpart,file="population4.csv",quote=FALSE,sep=",",row.names=FALSE)


#---------------
# End of Round. Beginning of next Round.
#---------------

# ---
# Add perturbation kernel
# ---
# get the stdev of each parameter and save for later
paramsd = sd(topdata[,1:5])

# -------
# RESET ROUND INFO HERE
# -------
write.table(t(paramsd),file="kernelsd5.csv",quote=FALSE,sep=",",row.names=FALSE)

# ---
# New draws 
# ---

newnum = 50000
newparams = data.frame(L=c(1),B=c(1),O=c(1),E=c(1),I=c(1))
ind = 1

while (ind<=newnum) {

   draws = sample(1:1000,1,replace=TRUE,prob=partweights)
   drawpart = topdata[draws,1:5]

   # and now the perturbation
   kerns = array(1:5)
   for (i in 1:5) {
      kerns[i] = runif(1,-paramsd[i],paramsd[i])
   }

   # and add them
   pertpart = drawpart + kerns

   # and check for boundary conditions
   if(!any(pertpart<0,pertpart>1,pertpart[5]<0.2)) {
      row.names(pertpart)[1]=ind
      newparams[ind,] = pertpart
      ind = ind + 1
   }
}

# add in other columns
fnums = formatC(0:49999,width=5,flag="0")
fnames = paste("Param",fnums,".txt",sep="")
fileout = cbind(c(0:49999),newparams,c(1),c(0),fnames)

# -------
# RESET ROUND INFO HERE
# -------
# write them to a file
write.table(fileout,file="sampleparams5.csv",quote=FALSE,sep=",",col.names=FALSE,row.names=FALSE)


