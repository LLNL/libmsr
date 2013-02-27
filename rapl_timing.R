#!/usr/bin/env Rscript
args = commandArgs(trailingOnly = TRUE)
if(length(args) < 1){
  cat("must provide an input file\n")
  quit()
}
a = read.table(args[1], header=T)

pkg_timestamp = a$timestamp[which(a$pkg_J > 0)]
pkg_sel = which(a$pkg_J > 0)
pkg_J = a$pkg_J[pkg_sel]
pkg_i = pkg_timestamp[2:length(pkg_timestamp)] - pkg_timestamp[1:(length(pkg_timestamp) - 1)]
pkg_P = pkg_J[2:length(pkg_J)] / pkg_i
plot(pkg_P)
dev.off();dev.new();
plot(pkg_i)
dev.off();dev.new();

pp0_timestamp = a$timestamp[which(a$pp0_J > 0)]
pp0_sel = which(a$pp0_J > 0)
pp0_i = pp0_timestamp[2:length(pp0_timestamp)] - pp0_timestamp[1:(length(pp0_timestamp) - 1)]
pp0_J = a$pp0_J[pp0_sel]
pp0_P = pp0_J[2:length(pp0_J)]/pp0_i
plot(pp0_i)
dev.off();dev.new();
hist(pp0_i);
dev.off();dev.new();
hist(pkg_i)
dev.off();dev.new();
plot(pkg_P);
dev.off();dev.new();
plot(pp0_P)
dev.off();dev.new();
ccf(pkg_P, pp0_P)
mean(pkg_timestamp - pp0_timestamp)
dev.off();dev.new();
ccf(pkg_i, pp0_i)
dev.off();dev.new();
hist(pkg_P)
dev.off();dev.new();
hist(pp0_P)
