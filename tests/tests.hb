x ~ Normal()
x ~ Normal("\"")
x ~ Normal('"')
x ~ Normal('j')
x ~ Normal("o")
x ~ Normal(0.0)
x ~ Normal(0.0,0.00)
x ~ Normal(0.0 , 0.00)
x ~ Normal(0,1)
x ~ Normal(0:0:[],1)
x ~ Normal(0+(0+0),1)
x ~ Normal(0==(0+0),1)
x ~ Normal((==),1)
x ~ Normal(0,"1")
x ~ Normal(0+0-0,1)
x ~ Normal(0+0-(-0),1)
x ~ Normal(x*y+z*w,1)
x ~ Normal(0 0,1)
y ~ Normal(x,1)
y ~ Normal([x],1)
y ~ Normal([x,x],1)
y ~ Normal((x,x),1)
y ~ Normal(N.x,1)
y ~ Normal((x),1)
y ~ Normal(x (z y),1)
y ~ Normal(x z y,1)
y ~ Normal((+) + (*),1)
y ~ Normal(fmap (+),1)
y ~ Normal(fmap (1+),1)
y ~ Normal(x ++ z ++ y,1)
y ~ Normal(x `seq` z `seq` y,1)
y ~ Normal((,,,) z (z y),1)
y ~ Normal(Maybe.mayb2e z (z y),1)
y ~ Normal(Mayb2e z (z y),1)
y ~ Normal(Maybe.Mayb2e z (z y),1)
y ~ Normal(fmap (\x->x+1) y,1)
y ~ Normal(if x then y else z,1)
y ~ Normal(\x->if x then y else z,1)
y ~ Normal(\x y->if x then y else z,1)
(x,y) ~ Normal(x)
[x,y] ~ Normal(x)
[x,y] ~ Normal(findFirst (\n->(targetNode t n)==n2) (edgesOutOfNode t n1))
[x,y] ~ Normal([edgeForNodes (n,n1) | n <- neighbors t n1, n /= n2])
