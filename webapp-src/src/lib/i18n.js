import i18n from '../js/i18next.js';
import Backend from '../js/i18nextXHRBackend.js';
import LanguageDetector from '../js/i18nextBrowserLanguageDetector.js';
import { reactI18nextModule } from 'react-i18next';

i18n
	.use(Backend)
	.use(LanguageDetector)
	.use(reactI18nextModule)
	.init({
		fallbackLng: 'en',

		// have a common namespace used around the full app
		ns: ['translations'],
		defaultNS: 'translations',

		interpolation: {
			escapeValue: false, // not needed for react!!
		},

		react: {
			wait: true
		},
    
    backend: {
      loadPath: 'locales/{{lng}}/{{ns}}.json'
    }
	});


export default i18n;
