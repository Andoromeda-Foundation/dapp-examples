var superagent = require('superagent');
var _ = require('lodash');
var async = require('async');

var nebulas = require("nebulas"),
   
    neb = new nebulas.Neb();
neb.setRequest(new nebulas.HttpRequest("https://testnet.nebulas.io")); //test


var netbegin = 'https://explorer.nebulas.io/test/api/';

export default (contract) => new Promise((resolve, reject) => {

});