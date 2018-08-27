import React from "react";
import { Icon, Button } from "antd";
import { withRouter } from "react-router-dom";
import intl from "react-intl-universal";

const IconStyle = {
    fontSize: "12rem",
    background: "#f5222d",
    color: "#FFF",
    borderRadius: "100%"
}

const buttonStyle = {
    margin: "0.5rem"
}

const navbarI18n = (name) => intl.get(`404.${name}`)

const PageNotFound = ({ location }) =>
    (
        <div id="page-not-found">
            <Icon type="exclamation" style={IconStyle} />
            <h1 className="title">{navbarI18n('title')}</h1>
            <h1 className="title"> {navbarI18n('code')} 404</h1>
            <h2 className="subtitle">
                {`${navbarI18n('The page you requested')} ${location.pathname} ${navbarI18n('is not exist')}`}
            </h2>
            <p className="description"> {navbarI18n('report-to-us')} If you were directed to here, please let us know! </p>
            <Button type="primary" size="large" style={buttonStyle}> {navbarI18n('Go Back')}</Button>
            <Button type="primary" size="large" style={buttonStyle}> {navbarI18n('Go Home')} </Button>
        </div>
    )

export default withRouter(PageNotFound)