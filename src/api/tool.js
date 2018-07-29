import Cookie from 'js-cookie';
import { BigNumber } from 'bignumber.js';
import request from 'superagent';

export class NasTool {
  static fromNasToWei(value) {
    return new BigNumber('1000000000000000000').times(value);
  }
  static fromWeiToNas(value) {
    if (value instanceof BigNumber) {
      return value.dividedBy('1000000000000000000');
    }
    return new BigNumber(value).dividedBy('1000000000000000000');
  }
}


export const getMe = async () => new Promise((resolve) => {
  window.postMessage({
    target: 'contentscript',
    data: {
    },
    method: 'getAccount',
  }, '*');
  window.addEventListener('message', ({ data }) => {
    if (data.data && data.data.account) {
      resolve(data.data.account);
    }
  });
});


export const getAnnouncements = async () => {
  const response = await request
    .get('https://api.leancloud.cn/1.1/classes/announcement')
    .set({
      'X-LC-Id': 'R6A46DH2meySCVNM1uWOoW2M-gzGzoHsz',
      'X-LC-Key': '8R6rGgpHa0Y9pq8uO53RAPCB',
    })
    .type('json')
    .accept('json');

  if (response.body && response.body.results) {
    return response.body.results;
  }

  return [];
};


export function getDrawCount(value, drawPrice) {
  let result = 0;
  let offset = 0;
  while (value > drawPrice + offset) {
    result += 1;
    value -= drawPrice + offset;
    offset += NasTool.fromNasToWei(0.0001);
  }
  return result;
}


export const getLocale = async () => (
  Cookie.get('locale') ||
  (
    navigator.language ||
    navigator.browserLanguage ||
    navigator.userLanguage
  ).toLowerCase()
);

export const setLocale = async (locale) => {
  Cookie.set('locale', locale, { expires: 365 });
};
