import Cookies from "js-cookie";

// Express them in One Line of code 😄
 
const getUserCrypto = () => (Cookies.get('crypto') || 'BTC')
const saveDefaultCrypto = (cryptoCode) => Cookies.set('crypto', cryptoCode)

const crypto = (state = getUserCrypto(), action) => {
    switch (action.type) {
        case 'ETH': {
            saveDefaultCrypto(action.type)
            return action.type
        }
        case 'BTC': {
            saveDefaultCrypto(action.type)
            return action.type
        }
        case 'EOS': {
            saveDefaultCrypto(action.type)
            return action.type
        }
        default: return state
    }
}

export default crypto