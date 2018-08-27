import { getter, setter } from "./cookieHandler";
import intl from "react-intl-universal";
import locales from "../locale";

function localeDetection() {
    const langs = window.navigator.languages
    const listOfLangs = langs.map(obj => obj.slice(0, 2))
    for (let lang of listOfLangs) {
        switch (lang) {
            case 'zh': return 'zh-CN'
            case 'ja': return 'ja-JP'
            case 'ko': return 'ko-KR'
            default: return 'en-US'
        }
    }
}

// Express them in One Line of code 😄
const name = "userLanguage"
const getUserLocale = getter({ name, defaultValue: localeDetection() })
const saveLocale = setter(name)

const initLang = (currentLocale) => intl.init({ currentLocale, locales })

const switchToLang = (code) => {
    saveLocale(code)
    initLang(code)
    return code
}

const lang = (state = getUserLocale, action) => {
    switch (action.type) {
        case 'SWITCH_TO_CHINESE': {
            return switchToLang('zh-CN')
        }
        case 'SWITCH_TO_JAPANESE': {
            return switchToLang('ja-JP')
        }
        case 'SWITCH_TO_KOREAN': {
            return switchToLang('ko-KR')
        }
        case 'SWITCH_TO_ENGLISH': {
            return switchToLang('en-US')
        }
        default: return state
    }
}

export default lang