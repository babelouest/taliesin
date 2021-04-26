import React, { Component } from 'react';

import StateStore from '../lib/StateStore';
import MediaInfo from './MediaInfo';
import i18n from '../lib/i18n';

class WebradioNext extends Component {
	constructor(props) {
		super(props);
		this.state = {
			imgThumbBlob: false, 
			media: props.media,
		};
		this.loadCover();
		
		StateStore.subscribe(() => {
      if (this._ismounted) {
        var reduxState = StateStore.getState();
        if (reduxState.lastAction === "showFullScreen") {
          if (!StateStore.getState().showFullScreen) {
            this.loadCover();
          }
        }
      }
		});
	}
	
	componentDidMount() {
		this._ismounted = true;
	}

	componentWillUnmount() {
		this._ismounted = false;
	}
	
	componentWillReceiveProps(nextProps) {
		var newMedia = (this.state.media.data_source !== nextProps.media.data_source || this.state.media.path !== nextProps.media.path);
		this.setState({media: nextProps.media}, () => {
			if (newMedia) {
				this.loadCover();
			}
		});
	}
	
	loadCover() {
		if (this.state.media && !StateStore.getState().showFullScreen && this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.media.data_source) + "/browse/path/" + encodeURI(this.state.media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
			.then((result) => {
				this.setState({imgThumbBlob: result});
			})
			.fail(() => {
				this.setState({imgThumbBlob: false});
			});
		}
	}
	
	render() {
		return (
			<MediaInfo media={this.state.media} imgThumbBlob={this.state.imgThumbBlob} meta={i18n.t("player.play_next")} index={-1}/>
		);
	}
}

export default WebradioNext;
