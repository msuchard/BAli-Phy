#!/usr/bin/make -f
SHELL=/bin/sh

TOPOLOGIES=c50 c66 c80 c90 c95 c99 c100

IMODEL=$(shell head -n 40 1.out | pickout --no-header 'indel model1')

PARTITIONS=$(shell grep data 1.out | sed "s/data\(.*\) = .*/\1/")

TREES=${TOPOLOGIES}

ifeq ($(origin CHAIN), undefined)
  CHAIN=1
endif

all: Results/analysis

#.DELETE_ON_ERROR: 

.PRECIOUS .SECONDARY: Results/c%.topology
.PRECIOUS .SECONDARY: Results/%
.PRECIOUS .SECONDARY: Work/%
.PRECIOUS .SECONDARY: Work/%.from.${SKIP}
.PRECIOUS .SECONDARY: Work/%.from.${SKIP}+${SIZE}
.PRECIOUS .SECONDARY: % 

.PRECIOUS .SECONDARY: Results/%-AU.prob
.PRECIOUS .SECONDARY: Results/distances%
.PRECIOUS .SECONDARY: Results/pairs%

Work/setup:
	@echo -e "Setting up directories... \c"
	@mkdir -p Work/
	@mkdir -p Mixing/
	@mkdir -p Results/
	@touch Work/setup
	@echo "done."


ifeq ($(IMODEL),)
AU_TARGETS=
else
#AU_TARGETS=$(foreach i,$(PARTITIONS),Results/MAP$(i)-AU.html Results/MAP$(i)-AU2.html Results/consensus-75-$(i).fasta Results/consensus-50-$(i).fasta Results/consensus-25-$(i).fasta Results/consensus-10-$(i).fasta Results/consensus-10-$(i)-AU.html Results/consensus-10-$(i)-AU2.html)
AU_TARGETS=$(foreach i,$(PARTITIONS),Results/MAP.html Results/consensus-10-1.html Results/consensus-75-$(i).fasta Results/consensus-50-$(i).fasta Results/consensus-25-$(i).fasta Results/consensus-10-$(i).fasta Results/consensus-10-$(i)-AU.html Results/consensus-10-$(i)-AU2.html Results/initial.html) Results/MAP-AU.html Results/MAP-AU2.html
endif


Results/analysis: Results/Report Results/consensus ${TOPOLOGIES:%=Results/%.topology} ${TREES:%=Results/%.tree} \
                  ${TREES:%=Results/%.mtree} ${TREES:%=Results/%-tree.pdf} Results/MAP.topology Results/MAP.tree \
                  Results/c-levels.plot Results/c-sub-levels.plot Work/trees.from.${SKIP}+${SIZE} \
		  Results/MAP.fasta Results/MAP.phy \
		  Mixing/partitions.SRQ Mixing/c50.SRQ Mixing/partitions.bs \
		  $(AU_TARGETS)
	@echo "Analyzed:  $$(wc -l < Work/trees.from.${SKIP}+${SIZE}) trees from iteration $$(( ${SKIP} + 1))." > Results/analysis
	@echo "Finished:  $$(date)" >> Results/analysis
	@echo 
	@cat Results/analysis
	@echo 

Results/Report:  Work/header Work/p Results/MAP.topology
	@{ cat Work/header ; \
	statreport 2: --skip ${SKIP} ${SIZE:%=--max %} < Work/p ; \
	cat Results/MAP.topology ; \
	} > $@

Work/header: Work/out Work/Pmarg
	@{ cat Work/Pmarg;\
	   echo; \
           tail -n +2 Work/out | head -n 100 | grep data;\
	   echo; \
           tail -n +2 Work/out | head -n 100 | grep model;\
           echo; } > $@

Work/samples%-${SKIP}+${SIZE}: Work/P%.fastas Work/setup
	@cut-range --skip ${SKIP} ${SIZE:%=--size %} < $< > $@

%.from.${SKIP}: %
	@tail -n +$$(( ${SKIP}+2 )) $< > $@

Work/%: ${CHAIN}.% Work/setup
	@cp ${CHAIN}.$* Work/$*

Work/%.from.${SKIP}+${SIZE}: Work/%.from.${SKIP}
	@head -n ${SIZE} $< > $@ 

Work/%.from.${SKIP}+: Work/%.from.${SKIP}
	@cp $< $@ 

Results/consensus : Work/trees Work/trees.from.${SKIP}+${SIZE}
	@echo -e "Summarizing topology distribution... \c";
	@trees-consensus --skip ${SKIP} ${SIZE:%=--max %}  ${MIN:%=--min-support %} $< --consensus 0.5,0.66,0.8,0.9,0.95,0.99,1.0 --sub-partitions > $@
	@echo "done."

Work/partitions: Results/consensus
	@pickout --no-header --large pi < $< > $@

Work/partitions.pred: Work/partitions
	@sed "s/$$/\n/" < $< > $@

Mixing/partitions.bs: Work/partitions.pred Work/trees
	@echo -e "Computing confidence intervals for partition support... \c";
	@trees-bootstrap --skip ${SKIP} ${SIZE:%=--max %} Work/trees --pred Work/partitions.pred > $@
	@echo "done."

Results/c-levels.plot : Results/consensus
	@echo -e "Generating consensus levels plot ... \c"
	@pickout --no-header LOD full < $< > $@
	@echo "done."

Results/c-sub-levels.plot : Results/consensus
	@echo -e "Generating consensus sub-levels plot ... \c"
	@pickout --no-header LOD sub\#1 < $< > $@
	@echo "done."

Results/%.tree : Results/%.topology Work/trees
	@echo "Calculating branch lengths for $* tree."
	@tree-mean-lengths $< --safe --skip ${SKIP} ${SIZE:%=--max %} < Work/trees > $@

Results/MAP.topology: Results/consensus Work/setup
	@pickout --no-header MAP-0 < $< > $@

Work/MAP-unordered.fasta: Work/MAP Work/setup
	@echo -e "Locating MAP alignment... \c"
	@alignment-find < Work/MAP > $@
	@echo "done."

Work/initial-unordered.fasta: Work/MAP Work/setup
	@echo -e "Locating MAP alignment... \c"
	@alignment-find --first < Work/MAP > $@
	@echo "done."

Work/median%-unordered.fasta: Work/samples%-${SKIP}+${SIZE}
	alignment-median --max-alignments 2000 --analysis=median< $< > $@

Results/%.fasta: Results/c50.tree Work/%-unordered.fasta
	@alignment-reorder Work/$*-unordered.fasta Results/c50.tree > $@ 

Results/au-graph%.plot Work/consensus-75-%-unordered.fasta: Work/samples%-${SKIP}+${SIZE}
	@echo -e "Computing consensus-75-$* alignment ... \c"
	@alignment-consensus --cutoff=0.75 --uncertainty=Results/au-graph.plot < Work/samples$*-${SKIP}+${SIZE} > $@ 
	@echo "done."

Results/au-graph%.plot Work/consensus-50-%-unordered.fasta: Work/samples%-${SKIP}+${SIZE}
	@echo -e "Computing consensus-50-$* alignment ... \c"
	@alignment-consensus --cutoff=0.50 --uncertainty=Results/au-graph.plot < Work/samples$*-${SKIP}+${SIZE} > $@
	@echo "done."

Results/au-graph%.plot Work/consensus-25-%-unordered.fasta: Work/samples%-${SKIP}+${SIZE}
	@echo -e "Computing consensus-25-$* alignment ... \c"
	@alignment-consensus --cutoff=0.25 --uncertainty=Results/au-graph.plot < Work/samples$*-${SKIP}+${SIZE} > $@
	@echo "done."

Results/au-graph%.plot Work/consensus-10-%-unordered.fasta: Work/samples%-${SKIP}+${SIZE}
	@echo -e "Computing consensus-10-$* alignment ... \c"
	@alignment-consensus --cutoff=0.10 --uncertainty=Results/au-graph.plot < Work/samples$*-${SKIP}+${SIZE} > $@
	@echo "done."

# Results/$file-$partition-AU.prob: Results/$file.fasta Work/samples$partitions-${SKIP}+${SIZE} Results/MAP.tree
Results/%-AU.prob: Results/%.fasta Work/samples1-${SKIP}+${SIZE} Results/MAP.tree
	@echo -e "Calculating AU probabilities for $* alignment ... \c"
	@alignment-gild Results/$*.fasta Results/MAP.tree --max-alignments 500 < Work/samples1-${SKIP}+${SIZE} > $@
	@echo "done."

Results/%-AU.html: Results/%-AU.prob Results/%.fasta
	@echo -e "Drawing $* alignment uncertainty plot ... \c"
	@alignment-draw Results/$*.fasta Results/$*-AU.prob > $@
	@echo "done."

Results/%-AU2.html: Results/%-AU.prob Results/%.fasta
	@echo -e "Drawing $* alignment uncertainty plot (2) ... \c"
	@if ! alignment-draw Results/$*.fasta Results/$*-AU.prob --color-scheme DNA+contrast+fade+fade+fade+fade > $@ ; then alignment-draw Results/$*.fasta Results/$*-AU.prob --color-scheme AA+contrast+fade+fade+fade+fade > $@ ; fi
	@echo "done."

Results/%.html: Results/%.fasta
	@echo -e "Drawing $* alignment ... \c"
	@if ! alignment-draw Results/$*.fasta --color-scheme DNA+contrast > $@ ; then alignment-draw Results/$*.fasta --color-scheme AA+contrast > $@ ; fi
	@echo "done."

Results/c%.topology: Results/consensus Work/setup
	@pickout --no-header $*-consensus < $< > $@ 
	@test -s $@

Results/c%.mtree: Results/consensus Work/setup
	@pickout --no-header --multi-line $*-consensus < $< > $@ 
	@test -s $@

Results/c%.mlengths: Results/c%.mtree Work/trees
	@mctree-mean-lengths $< --skip ${SKIP} ${SIZE:%=--max %} < Work/trees > $@

Results/c%-tree.dot: Results/c%.mtree
	@draw-graph $< > $@

Results/%.pdf: Results/%.dot
	@neato -Tpdf $< > $@

Mixing/partitions.SRQ: Work/trees Work/partitions.pred
	@echo -e "Generating SRQ plot for c50 partitions... \c"
	@trees-to-SRQ Work/partitions.pred --skip ${SKIP} ${SIZE:%=--max %} < Work/trees> $@
	@echo "done."

Mixing/%.SRQ: Work/trees Results/%.topology
	@echo -e "Generating SRQ plot for $* tree... \c"
	@trees-to-SRQ Results/$*.topology --skip ${SKIP} ${SIZE:%=--max %} < Work/trees> $@
	@echo "done."

Mixing/sum.plot: Work/trees Work/partitions.pred
	@echo -e "Generating sum plot for c50 partitions... \c"
	@trees-to-SRQ Work/partitions.pred  --skip ${SKIP} ${SIZE:%=--max %} < Work/trees--mode sum --no-scale-x > $@
	@echo "done."

Mixing/sumi.plot: Work/trees Work/partitions.pred
	@echo -e "Generating inverse sum plot for c50 partitions... \c"
	@trees-to-SRQ Work/partitions.pred  --invert --skip ${SKIP} ${SIZE:%=--max %} < Work/trees --mode sum --no-scale-x > $@
	@echo "done."

Work/Likelihood: Work/p Work/setup
	@stats-select --no-header likelihood < Work/p > $@

Work/Pmarg: Work/Likelihood.from.${SKIP}+${SIZE}
	@echo -e "Calculating marginal likelihood... \c"
	@model_P < $< > $@ 
	@echo "done."

clean:
	rm -rf Work/ Results/ Mixing/

%.fasta: %.phy
	@alignment-convert --output fasta < $< > $@

%.phy: %.fasta
	@alignment-convert --output phylip < $< > $@

Results/distances%: Work/samples%-${SKIP}+${SIZE} Work/MAP-unordered.fasta
	@echo -e "Calculating alignment distances to MAP... \c"
	@fixedpt-alignment-distances Work/MAP-unordered.fasta < Work/samples$*-${SKIP}+${SIZE} > $@

Results/pairs%: Work/samples%-${SKIP}+${SIZE} 
	@echo -e "Calculating pairwise distances between posterior alignment sample... \c"
	@pairwise-alignment-distances ${THINTO} < Work/samples$*-${SKIP}+${SIZE} > $@
