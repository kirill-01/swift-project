#!/bin/bash

killbypid() {
  pgrep ${1} > /tmp/pids.swift
  cat /tmp/pids.swift
  while read p; do
          kill -9 ${p}
  done </tmp/pids.swift
  rm -rf /tmp/pids.swift
}

killbypid swift-
killbypid crossbar

swift-gui

killbypid swift-
killbypid crossbar
