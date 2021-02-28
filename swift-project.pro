TEMPLATE = subdirs

SUBDIRS = qmsgpack \
          swift-corelib \
          swift-server \
          # swift-cexio \
          swift-bittrex \
          swift-bitfinex \
          swift-binance \
          swift-hitbtc \
          swift-zb \
          swift-kucoin \
          swift-huobi \
          swift-kraken \
          swift-idcm \
          # swift-livecoin \
          swift-orderbooks \
          swift-arbitrage \
          swift-balance \
          swift-orders \
          swift-telegram \
          swift-wampcfg \
          swift-console \
          # swift-pamm \
          # swift-blockcypher \
          # swift-gui

CONFIG += ordered
