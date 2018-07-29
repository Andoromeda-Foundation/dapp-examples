import React from "react";
import intl from "react-intl-universal";
const i18n = (name) => intl.get(`construction.${name}`)

const A11yEmoji = ({ emoji, label, style }) => <span role="img" aria-label={label} style={style}> {emoji} </span>

const inConstruction = ({ pageName }) => (
    <div className="in-construction">
        <A11yEmoji emoji="⚠️" style={{ fontSize: "6rem" }} label="warning" />
        <h1 className="title">
            <A11yEmoji emoji="⚠️" label="warning" />
            <A11yEmoji emoji="🔨" label="hammer" />
            「{pageName}」{i18n("in custruction")}
            <A11yEmoji emoji="🔨" label="hammer" />
            <A11yEmoji emoji="⚠️" label="warning" />
        </h1>
        <p className="subtitle"> ,
            {i18n("checkout later")} </p>
    </div>
)

export default inConstruction