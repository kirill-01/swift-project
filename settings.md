```ini

# Arbitrage module
[arbitrage]
binary=/opt/swift-bot/bin/swift-arbitrage
description=Arbitrage trading module
is_enabled=true
is_exchange=false
max_order_size=5.00  // Maximum order size ( in BTC! )
min_order_size=0.15 // Minimum order size ( in BTC! )
name=arbitrage
stats_show_interval=3600000 // Publish statistic interval
step_order_size=0.02 // Incrementing order candidat step ( in BTC! )
usage_report_interval=150 // Log filter usage report every n times

[balance]
balance_update_interval=30000
binary=/opt/swift-bot/bin/swift-balance
deposits_update_interval=360000
description=balance module
is_enabled=true
is_exchange=false
name=balance
requirements_update_interval=360000
use_only_exchange_account=true
withdraws_update_interval=420000

[binance]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-binance
description=Binance API client module
is_enabled=true
is_exchange=true
name=binance
version=1.0.252
wamp_password=298D254629CE
wamp_role=mbinance
wamp_user=binance

[bitfinex]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-bitfinex
description=bitfinex API client module
is_enabled=true
is_exchange=true
name=bitfinex
requests_delay=350
requests_repeat_delay=500
version=1.0.252
wamp_password=1A45C57AAD13
wamp_role=mbitfinex
wamp_user=bitfinex

[bittrex]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-bittrex
description=Bittrex API client module
is_enabled=true
is_exchange=true
name=bittrex
version=1.0.252
wamp_password=705E00043747
wamp_role=mbittrex
wamp_user=bittrex

[console]
name=console
version=1.0.255
wamp_password=5C5EEE0131B9
wamp_role=mconsole
wamp_user=console

[hitbtc]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-hitbtc
description=HitBTC API client module
is_enabled=true
is_exchange=true
name=hitbtc
version=1.0.252
wamp_password=40A892DDD2DB
wamp_role=mhitbtc
wamp_user=hitbtc

[huobi]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-huobi
description=huobi API client module
is_enabled=true
is_exchange=true
name=huobi
version=1.0.252
wamp_password=308B46E82E42
wamp_role=mhuobi
wamp_user=huobi

[idcm]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-idcm
description=IDCM API client module
is_enabled=true
is_exchange=true
name=idcm
version=1.0.269
wamp_password=BD528EAFA16B
wamp_role=midcm
wamp_user=idcm

[kraken]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-kraken
description=kraken API client module
is_enabled=true
is_exchange=true
name=kraken
version=1.0.269
wamp_password=7F581FECC8E7
wamp_role=mkraken
wamp_user=kraken

[kucoin]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-kucoin
description=KuCoin API client module
is_enabled=true
is_exchange=true
name=kucoin
version=1.0.269
wamp_password=5D8B78C372E2
wamp_role=mkucoin
wamp_user=kucoin

[livecoin]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-livecoin
cache_orderbooks=true
cache_orderbooks_time=1500
description=livecoin API client module
is_enabled=false
is_exchange=true
name=livecoin
version=1.0.212
wamp_password=441633693A58
wamp_role=mlivecoin
wamp_user=livecoin

[orderbooks]
binary=/opt/swift-bot/bin/swift-orderbooks
description=OrderBooks proxy aggregator
is_enabled=true
is_exchange=false
limit_records_cnt=20
name=orderbooks
orderbooks_valid_time=3000
publish_interval=850
request_interval=1000
send_rates_interval=360000
version=1.0.252
wamp_password=1FBB1722472A
wamp_role=morderbooks
wamp_user=orderbooks

[orders]
active_update_interval=30000
binary=/opt/swift-bot/bin/swift-orders
description=orders module
history_update_interval=360000
is_enabled=true
is_exchange=false
name=orders
version=1.0.252
wamp_password=78DE7C137462
wamp_role=morders
wamp_user=orders

[securewamp]
binary=/opt/swift-bot/bin/swift-securewamp
description=Auth component for WAMP crossbar router
is_enabled=false
is_exchange=false
name=securewamp
version=1.0.0
wamo_secret=secret123
wamp_user=authenticator1

[server]
name=server
version=1.0.269
wamp_password=1BAA6161593D
wamp_role=mserver
wamp_user=server

[settings]
api_debug=false
clickhouse_db=
clickhouse_enabled=false
clickhouse_host=
clickhouse_password=
clickhouse_port=
clickhouse_user=
crossbar_binary=/home/kkuznetsov/.local/bin/crossbar
debug_outputs=false
default_currencies="usdt,eth,btc,xrp"
errors_output=true
exclude_pairs="ETH-BTC,XRP-BTC,XRP-ETH,ETH-XRP"
hostinfo_interval=45000
logs_output=true
max_log_events=1000
max_logfile_size_mb=1024
modules_debug=false
mysql_db=swiftbot
mysql_host=localhost
mysql_password=
mysql_port=3306
mysql_user=root
reports_outputs=true
startup_secured=true
wamp_debug=false
wamp_home=localhost
wamp_port=8081
wamp_realm=realm1
warnings_outputs=true

[telegram]
binary=/opt/swift-bot/bin/swift-telegram
chat_id=1019332177
description=Telegram API module
is_enabled=true
is_exchange=false
name=telegram
telegram_api=1395314851:AAGlakzocjMGW2c9IPpwGDuJt4bmdSd1msA
version=1.0.252
wamp_password=4D7B3458C8EF
wamp_role=mtelegram
wamp_user=telegram

[wampcfg]
binary=/opt/swift-bot/bin/swift-wampcfg
description=Build, manage crossbar config rules
is_enabled=false
is_exchange=false
name=wampcfg
version=1.0.75

[zb]
api_key=
api_secret=
api_user=
binary=/opt/swift-bot/bin/swift-zb
description=ZB API client module
is_enabled=true
is_exchange=true
name=zb
version=1.0.269
wamp_password=691E7705D14E
wamp_role=mzb
wamp_user=zb
```
