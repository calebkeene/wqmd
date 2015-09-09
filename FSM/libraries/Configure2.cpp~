// example
float y[4]={3, 4, 5, 6};
float x[4]={2.9875,3.9937,4.9925,5.9925};
float lrCoef[2]={0,0};

void setup(){
  Serial.begin(9600);
  delay(1000);
  
  // call simple linear regression algorithm
  simpLinReg(x, y, lrCoef, 4);
  
  Serial.print(lrCoef[0],8);
  Serial.print(" ");
  Serial.println(lrCoef[1],8);
}


void loop(){
}


void simpLinReg(float* x, float* y, float* lrCoef, int n){
  // pass x and y arrays (pointers), lrCoef pointer, and n.  The lrCoef array is comprised of the slope=lrCoef[0] and intercept=lrCoef[1].  n is length of the x and y arrays.
  // http://en.wikipedia.org/wiki/Simple_linear_regression

  // initialize variables
  float xbar=0;
  float ybar=0;
  float xybar=0;
  float xsqbar=0;
  
  // calculations required for linear regression
  for (int i=0; i<n; i++){
    xbar=xbar+x[i];
    ybar=ybar+y[i];
    xybar=xybar+x[i]*y[i];
    xsqbar=xsqbar+x[i]*x[i];
  }
  xbar=xbar/n;
  ybar=ybar/n;
  xybar=xybar/n;
  xsqbar=xsqbar/n;
  
  // simple linear regression algorithm
  lrCoef[0]=(xybar-xbar*ybar)/(xsqbar-xbar*xbar);
  lrCoef[1]=ybar-lrCoef[0]*xbar;
}
