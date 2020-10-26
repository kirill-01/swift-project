#!/bin/bash

[ ${1} = "eae" ] && {

EXCHANGES=(bittrex bitfinex binance hitbtc zb kucoin huobi kraken idcm livecoin)

for item in ${EXCHANGES[*]}
do
    sed -i "s/is_enabled=false/is_enabled=true/" /opt/swift-bot/modules/${item}/${item}.ini
done

}

[ ${1} = "da" ] && {
    find /opt/swift-bot/modules/ -type f -name '*.ini' -exec sed -i "s/is_enabled=true/is_enabled=false/" {} \;
}

[ ${1} = "ee" ] && {
    sed -i "s/is_enabled=false/is_enabled=true/" /opt/swift-bot/modules/${2}/${2}.ini
}

[ ${1} = "de" ] && {
    sed -i "s/is_enabled=true/is_enabled=false/" /opt/swift-bot/modules/${2}/${2}.ini
}
