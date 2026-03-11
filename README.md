# rate-limiter-system
# Rate Limiter System

## Algorithms Implemented
1. Fixed Window
2. Token Bucket

## Features
- Thread-safe using std::mutex
- Supports multiple clients
- Multithreaded request simulation
- Per-request logging
- Summary statistics

## Build

g++ main.cpp -pthread

## Run

./a.out

## Design

Each client has independent rate limit state stored in an unordered_map.

Mutex ensures thread-safe updates.

## Tradeoffs

Fixed Window allows burst at window boundary.

Token Bucket smooths traffic by distributing tokens over time.
