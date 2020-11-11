# Extremely fast and scalable standalone cryptocurrency trading platform

A compleate sulution to access market data, implement trading strategies or anything else, what you need.

It is intended to be used by coders, developers, technically-skilled traders, data-scientists and financial analysts for building trading algorithms.

## Safety first

- Manage access rights for each module.
- Use safe start option, to generate new auth credentials for modules before server start.
- No external code depencies.

## Speed really matters

- Async API clients without response awaiters
- Arbitrage variants calculation by orderbooks for 2 markets on 12 exchanges takes about 125 msecs! ( Including all fees calculations )
- Each module ( API client, for example ) running in their own process

## Fault safe

- Microservices architecture. Start, stop, restart any module at any time
- Change any config options at runtime

### [Api reference](docs/api.md)

