package main

import (
	"fmt"
	"log"
	"math/big"
	"math/rand"
	"os"
	"runtime"
	"strconv"
	"sync/atomic"
	"time"
)

var (
	zero   = big.NewInt(0)
	one    = big.NewInt(1)
	two    = big.NewInt(2)
	three  = big.NewInt(3)
	five   = big.NewInt(5)
	seven  = big.NewInt(7)
	eleven = big.NewInt(11)
)

func isProbablePrime(n *big.Int) bool {
	if new(big.Int).Mod(n, three).Cmp(zero) == 0 ||
		new(big.Int).Mod(n, five).Cmp(zero) == 0 ||
		new(big.Int).Mod(n, seven).Cmp(zero) == 0 ||
		new(big.Int).Mod(n, eleven).Cmp(zero) == 0 {
		return false
	}

	nMinus1 := new(big.Int).Sub(n, one)
	if new(big.Int).Exp(two, nMinus1, n).Cmp(one) != 0 {
		return false
	}

	d := new(big.Int).Set(nMinus1)
	r := 0
	for d.Bit(0) == 0 {
		d.Rsh(d, 1)
		r++
	}

	x := new(big.Int).Exp(two, d, n)
	if x.Cmp(one) == 0 || x.Cmp(nMinus1) == 0 {
		return true
	}

	for i := 0; i < r-1; i++ {
		x.Mul(x, x).Mod(x, n)
		if x.Cmp(nMinus1) == 0 {
			return true
		}
	}
	return false
}

func findPrimeWorker(startOffset int64, stride int64, lowerBound, upperBound *big.Int, found *int32) *big.Int {
	candidate := new(big.Int).Set(lowerBound)
	offset := new(big.Int).SetInt64(startOffset * 2)
	candidate.Add(candidate, offset)

	if candidate.Bit(0) == 0 {
		candidate.Add(candidate, one)
	}

	strideBy := new(big.Int).SetInt64(stride * 2)

	for candidate.Cmp(upperBound) <= 0 && atomic.LoadInt32(found) == 0 {
		if isProbablePrime(candidate) {
			nMinus1 := new(big.Int).Sub(candidate, one)
			if new(big.Int).Exp(three, nMinus1, candidate).Cmp(one) == 0 {
				if atomic.CompareAndSwapInt32(found, 0, 1) {
					return new(big.Int).Set(candidate)
				}
			}
		}
		candidate.Add(candidate, strideBy)
	}
	return nil
}

func generateLargePrime(digits int) (*big.Int, error) {
	if digits <= 1 {
		return nil, fmt.Errorf("number of digits must be greater than 1")
	}

	lowerBound := new(big.Int).Exp(big.NewInt(10), big.NewInt(int64(digits-1)), nil)
	upperBound := new(big.Int).Exp(big.NewInt(10), big.NewInt(int64(digits)), nil)

	rangeSize := new(big.Int).Sub(upperBound, lowerBound)
	startOffset := new(big.Int)
	startOffset.Rand(rand.New(rand.NewSource(time.Now().UnixNano())), rangeSize)
	searchStart := new(big.Int).Add(lowerBound, startOffset)
	if searchStart.Bit(0) == 0 {
		searchStart.Add(searchStart, one)
	}

	numWorkers := runtime.NumCPU() * 2
	fmt.Printf("Using %d workers for parallel search...\n", numWorkers)

	var found int32
	results := make(chan *big.Int, numWorkers)

	for i := 0; i < numWorkers; i++ {
		go func(workerID int) {
			if prime := findPrimeWorker(int64(workerID), int64(numWorkers), searchStart, upperBound, &found); prime != nil {
				results <- prime
			}
		}(i)
	}

	for i := 0; i < numWorkers; i++ {
		go func(workerID int) {
			backStart := new(big.Int).Sub(upperBound, big.NewInt(int64(workerID*2)))
			if backStart.Bit(0) == 0 {
				backStart.Sub(backStart, one)
			}

			candidate := new(big.Int).Set(backStart)
			strideBy := new(big.Int).SetInt64(int64(numWorkers * 2))

			for candidate.Cmp(searchStart) >= 0 && atomic.LoadInt32(&found) == 0 {
				if isProbablePrime(candidate) {
					nMinus1 := new(big.Int).Sub(candidate, one)
					if new(big.Int).Exp(three, nMinus1, candidate).Cmp(one) == 0 {
						if atomic.CompareAndSwapInt32(&found, 0, 1) {
							results <- new(big.Int).Set(candidate)
							return
						}
					}
				}
				candidate.Sub(candidate, strideBy)
			}
		}(i)
	}

	select {
	case prime := <-results:
		atomic.StoreInt32(&found, 1)
		return prime, nil
	case <-time.After(30 * time.Second):
		return nil, fmt.Errorf("timeout finding prime")
	}
}

func main() {
	numDigits := 1000
	if len(os.Args) > 1 {
		parsedDigits, err := strconv.Atoi(os.Args[1])
		if err != nil {
			log.Fatalf("Invalid number of digits: %v", err)
		}
		numDigits = parsedDigits
	}

	fmt.Printf("--- Finding a %d-digit prime ---\n", numDigits)
	startTime := time.Now()
	prime, err := generateLargePrime(numDigits)
	duration := time.Since(startTime)

	if err != nil {
		log.Fatalf("Failed to generate prime: %v", err)
	}

	fmt.Println("\nFound prime:")
	fmt.Println(prime.String())
	fmt.Printf("\nDigits: %d\n", len(prime.String()))
	fmt.Printf("Time: %s\n", duration)
}

