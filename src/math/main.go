package main

import (
	"math"
)

const ( // TODO: parse from flags
	xStart = 0
	xStop  = 2.7
	xStep  = 0.02
)

func mapFunc[T, S any](data []T, f func(T) S) []S {
	res := make([]S, 0, len(data))

	for _, val := range data {
		res = append(res, f(val))
	}

	return res
}

func reduceSteps[T, S any](data []T, f func(S, T) S, initValue S) []S {
	res := make([]S, 0, len(data))
	acc := initValue

	for _, val := range data {
		acc = f(acc, val)
		res = append(res, acc)
	}

	return res
}

var (
	mapFuncFloat64     = mapFunc[float64, float64]
	reduceStepsFloat64 = reduceSteps[float64, float64]
)

type task struct{}

func (t task) picardFirstRank(x float64) float64 {
	return math.Pow(x, 3) / 3
}

func (t task) picardSecondRank(x float64) float64 {
	return math.Pow(x, 3)/3 +
		math.Pow(x, 7)/63
}

func (t task) picardThirdRank(x float64) float64 {
	return math.Pow(x, 3)/3 +
		math.Pow(x, 7)/63 +
		2*math.Pow(x, 11)/2079 +
		math.Pow(x, 15)/59535
}

func (t task) picardFourthRank(x float64) float64 {
	return math.Pow(x, 3)/3 +
		math.Pow(x, 7)/63 +
		2*math.Pow(x, 11)/2079 +
		13*math.Pow(x, 15)/218295 +
		82*math.Pow(x, 19)/37328445 +
		662*math.Pow(x, 23)/10438212015 +
		4*math.Pow(x, 27)/3341878155 +
		math.Pow(x, 31)/109876902975
}

func (t task) targetFunction(x, y float64) float64 {
	return x*x + y*y
}

func (t task) eulerExplicitFunc(y, x float64) float64 {
	return y + xStep*t.targetFunction(x, y)
}

func (t task) eulerImplicitFunc(y, x float64) float64 {
	return solveEquation(y, func(y1 float64) float64 {
		return y1 - xStep*t.targetFunction(x+xStep, y1) - y
	})
}

func (t task) solve(xSlice []float64) []float64 {
	yPicardFirstRank := mapFuncFloat64(xSlice, t.picardFirstRank)
	yPicardSecondRank := mapFuncFloat64(xSlice, t.picardSecondRank)
	yPicardThirdRank := mapFuncFloat64(xSlice, t.picardThirdRank)
	yPicardFourthRank := mapFuncFloat64(xSlice, t.picardFourthRank)
	yEulerExplicit := reduceStepsFloat64(xSlice, t.eulerExplicitFunc, 0)
	yEulerImplicit := reduceStepsFloat64(xSlice, t.eulerImplicitFunc, 0)

	res := append(yPicardFirstRank, yPicardSecondRank...)
	res = append(res, yPicardThirdRank...)
	res = append(res, yPicardFourthRank...)
	res = append(res, yEulerExplicit...)
	res = append(res, yEulerImplicit...)

	return res
}

func solveEquation(x0 float64, f func(float64) float64) float64 {
	step := 0.00001
	df := (f(x0+step) - f(x0)) / step
	for i := 0; i < 1000; i++ {
		x0 = x0 - f(x0)/df
	}

	return x0
}

func main() {
	xSlice := make([]float64, 0, int(math.Trunc((xStop-xStart)/xStep))+1)
	xSlice = append(xSlice, xStart)
	for i := 1; i < cap(xSlice); i++ {
		xSlice = append(xSlice, xSlice[i-1]+xStep)
	}
	for {
		res := task{}.solve(xSlice)
		_ = res
	}
}
