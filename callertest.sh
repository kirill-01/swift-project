#!/bin/bash
curl -H "Content-Type: application/json" \
    -d '{"procedure": "swift.telegram.callback", "args": [1, 2]}' \
    https://swiftbot.ru/callback

