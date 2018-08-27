var superagent = require('superagent');
var _ = require('lodash');
var async = require('async');

var nebulas = require("nebulas"),
    // Account = nebulas.Account,
    neb = new nebulas.Neb();
neb.setRequest(new nebulas.HttpRequest("https://testnet.nebulas.io")); //test
// neb.setRequest(new nebulas.HttpRequest("https://mainnet.nebulas.io")); //main

var netbegin = 'https://explorer.nebulas.io/test/api/';
// var NebPay = require("nebpay.js");     //https://github.com/nebulasio/nebPay
// var nebPay = new NebPay();

// // var callbacks = NebPay.config.testnetUrl;
// var callbacks = NebPay.config.mainnetUrl;   //如果合约在主网,则使用这个
// var contract = '';

export default (contract) => new Promise((resolve, reject) => {

    var page = 1;
    var fetchUrl = netbegin + `tx?a=${contract}&p=${page}`

    superagent.get(fetchUrl).end(async (err, res) => {

        var totalPage = 0;
        try{
            totalPage = res.body.data.totalPage;
        }catch(e){
            console.log(e);
        }
        var buylist = [];
        
        var txArr = [];
        var arr = _.fill(Array(totalPage), 1);
        var index = 0;

        async.eachSeries(arr, (acc, callback) => {

            index++;
            var url = netbegin + `tx?a=${contract}&p=${index}`

            superagent.get(url).end((err, res) => {

                var txnList = [];
                try{
                    txnList = res.body.data.txnList
                }catch(e){

                }
                _.each(txnList, async (tx) => {
                    var _tx = {
                        address: tx.from.hash,
                        balance: tx.from.balance / 10 ** 18
                    }

                    var func = JSON.parse(tx.data).Function;
                    
                    var one = {};
                    one.key = buylist.length + 1;
                    one.event = func;
                    one.player = tx.from.hash;
                    one.price = tx.value / 10**18;
                    one.amount = "-";
                    one.timesecond = tx.timestamp / 1000;
                    one.time = new Date(one.timesecond * 1000).toLocaleString();
                    buylist.unshift(one);
                        
                    await new Promise((resolve, reject) => { neb.api.getEventsByHash({hash: tx.hash})
                        .then((events) => {
                            one.amount = JSON.parse(events.events[2].data).Transfer.amount;
                            resolve(events)
                        }).catch((e) => {
                        });
                    });

                    txArr.push(_tx);
                })

                setTimeout(function () {
                    callback(err);
                }, 100);

            });

        }, (err) => {
            
            resolve(buylist);

        })
        
    });
    
});