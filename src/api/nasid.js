import axios from "axios";
import { notification } from "antd";
const contractAddress = 'n1g3dNR43HN62cT8i72wGt747Z55dBYv1im'
const api = 'https://mainnet.nebulas.io'

async function call({
    from, //
    functionName,
    value = '0',
    args = [],
}) {
    // const { contractAddress, api } = this;
    const to = contractAddress;
    const txParams = {
        value,
        nonce: 0,
        gasPrice: '1000000',
        gasLimit: '2000000',
        contract: { function: functionName, args: JSON.stringify(args) },
    };
    const { data } = await axios
        .post(`${api}/v1/user/call`, (Object.assign({ from, to }, txParams)));

    return data.result.result;
}

export default async function fetchAccountDetail(address) {
    if (address === null) {
        return null;
    }

    try {
        const  result  = await call({ from: address, functionName: 'get' });
        if (typeof result === 'string') {
            return JSON.parse(result);
        } else {
            return {}
        }
    } catch (error) {
        notification.warning({
            title: 'Nebulas Sucks'
        })
    }
}

