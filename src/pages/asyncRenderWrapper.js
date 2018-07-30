import React from "react";
import Loadable from 'react-loadable';

const LoadingMessage = (name) => (<div> Loading {name} modules, Please wait</div>)

export const Faq = Loadable({
  loader: () => import ('./Faq'),
  loading: () => LoadingMessage('Faq')
})

export const Home = Loadable({
  loader: () => import('../containers/Home'),
  loading: () => LoadingMessage('Home')
})

export const Eth = Loadable({
  loader: () => import ('./Eth'),
  loading: () => LoadingMessage('Eth')
});
