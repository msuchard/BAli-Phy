module Prelude where
{
h:t !! 0 = h;
h:t !! i = t !! (i-1);

fmap1 f [] = [];
fmap1 f ((x,y):l) = (f x,y):(fmap1 f l);
fmap1 f (DiscreteDistribution l) = DiscreteDistribution (fmap1 f l);

fmap2 f [] = [];
fmap2 f ((x,y):l) = (x,f y):(fmap2 f l);
fmap2 f (DiscreteDistribution l) = DiscreteDistribution (fmap2 f l);

extendDiscreteDistribution (DiscreteDistribution d) p x = DiscreteDistribution (p,x):(fmap1 (\q->q*(1.0-p)) d);

sum  = foldl' (+) 0;

average (DiscreteDistribution l) = foldl' (\x y->(x+(fst y)*(snd y))) 0.0 l;
uniformDiscretize q n = let {n' = (intToDouble n)} in DiscreteDistribution (map (\i->(1.0/n',q (2.0*i+1.0)/n')) (take n [0..]));

enumFrom x = x:(enumFrom (x+1));
enumFromTo x y = if (x==y) then [x] else x:(enumFromTo (x+1) y);

zip (x:xs) (y:ys) = (x,y):(zip xs ys);
zip []   _        = [];
zip _   []        = [];

concat xs = foldr (++) [] xs;

concatMap f = concat . map f;

length l = foldl' (\x y ->(x+1)) 0 l;

listArray n l = mkArray n (\i -> l !! i);

listArray' l = listArray (length l) l;

uniformQuantiles q n = map (\i -> q ((2.0*(intToDouble i)+1.0)/(intToDouble n)) ) (take n [1..]);

unwrapDD (DiscreteDistribution l) = l;

mixDiscreteDistributions' (h:t) (h2:t2) = DiscreteDistribution (fmap1 (\q->q*h) h2)++(mixDiscreteDistributions' t t2);
mixDiscreteDistributions' [] [] = [];

mixDiscreteDistributions l1 l2 = DiscreteDistribution (mixDiscreteDistributions' l1 (fmap unwrapDD l2));

listFromVectorInt' v s i = if (i<s) then (getVectorIntElement v i):listFromVectorInt' v s (i+1) else [];

listFromVectorInt v = listFromVectorInt' v (sizeOfVectorInt v) 0;

listFromString' v s i = if (i<s) then (getStringElement v i):listFromString' v s (i+1) else [];

listFromString v = listFromString' v (sizeOfString v) 0;

listFromVectorVectorInt' v s i = if (i<s) then (getVectorVectorIntElement v i):listFromVectorVectorInt' v s (i+1) else [];

listFromVectorVectorInt v = listFromVectorVectorInt' v (sizeOfVectorVectorInt v) 0;

listFromVectorvectorInt' v s i = if (i<s) then (getVectorvectorIntElement v i):listFromVectorvectorInt' v s (i+1) else [];

listFromVectorvectorInt v = listFromVectorvectorInt' v (sizeOfVectorvectorInt v) 0;

newVectorInt s = IOAction1 builtinNewVectorInt s;

setVectorIndexInt v i x = IOAction3 builtinSetVectorIndexInt v i x;

copyListToVectorInt [] v i = return ();
copyListToVectorInt (h:t) v i = do {setVectorIndexInt v i h; copyListToVectorInt t v (i+1)};

listToVectorInt l = unsafePerformIO (do {v <- newVectorInt (length l); copyListToVectorInt l v 0; return v});

newString s = IOAction1 builtinNewString s;

setStringIndexInt v i x = IOAction3 builtinSetStringIndexInt v i x;

copyListToString [] v i = return ();
copyListToString (h:t) v i = do {setStringIndexInt v i h ; copyListToString t v (i+1)};

listToString l = unsafePerformIO (do {v <- newString (length l); copyListToString l v 0; return v});

newVectorDouble s = IOAction1 builtinNewVectorDouble s;
setVectorIndexDouble v i x = IOAction3 builtinSetVectorIndexDouble v i x;
copyListToVectorDouble [] v i = return ();
copyListToVectorDouble (h:t) v i = do { setVectorIndexDouble v i h ; copyListToVectorDouble t v (i+1)};

listToVectorDouble l = unsafePerformIO (do { v <- newVectorDouble (length l); copyListToVectorDouble l v 0; return v});

newVectorMatrix s = IOAction1 builtinNewVectorMatrix s;
setVectorIndexMatrix v i x = IOAction3 builtinSetVectorIndexMatrix v i x;
copyListToVectorMatrix [] v i = return ();
copyListToVectorMatrix (h:t) v i = do { setVectorIndexMatrix v i h; copyListToVectorMatrix t v (i+1)};

listToVectorMatrix l = unsafePerformIO (do { v <- newVectorMatrix (length l); copyListToVectorMatrix l v 0 ; return v});

error m = builtinError (listToString m);

fail e = error e
}
