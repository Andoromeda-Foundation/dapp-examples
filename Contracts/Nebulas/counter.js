/**
 * CryptoHero Contract Nebulas Version
 * ©️ Andoromeda Foundation All Right Reserved.
 * @author: MinakoKojima <lychees67@gmail.com>
 * @version: 1.0
 */

"use strict"

class Operator {
    constructor(obj) {
        this.operator = {}
        this.parse(obj)
    }

    toString() {
        return JSON.stringify(this.operator)
    }

    parse(obj) {
        if (typeof obj != "undefined") {
            var data = JSON.parse(obj)
            for (var key in data) {
                this.operator[key] = data[key]
            }
        }
    }

    get(key) {
        return this.operator[key]
    }

    set(key, value) {
        this.operator[key] = value
    }
}

class CounterContract {
    constructor() {       
        LocalContractStorage.defineProperties(this, {
            counter: null
        })
        LocalContractStorage.defineMapProperties(this, {
        })
    }

    init() {
        this.counter = 0
    }

    inc() {
        this.counter += 1
    }

    get() {
        return this.counter
    }
}

module.exports = CounterContract