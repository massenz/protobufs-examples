// Copyright (c) 2016 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 3/6/16.

#pragma once

#include <string>
#include <glog/logging.h>

namespace {

class Sample {
  // Insert your classes definitions here.
 private:
  std::string sample_;

 public:
  explicit Sample(const std::string name) : sample_ {name} { };

  std::string name() const { return sample_; }
};
}
