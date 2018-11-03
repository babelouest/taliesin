import React, { Component } from 'react';

import StateStore from '../lib/StateStore';
import MediaInfo from './MediaInfo';
import i18n from '../lib/i18n';

class WebradioNow extends Component {
	constructor(props) {
		super(props);
		this.state = {
			imgThumbBlob: false, 
			media: props.media,
		};

		this.loadCover();
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "showFullScreen") {
				if (!StateStore.getState().showFullScreen) {
					this.loadCover();
				}
			}
		});
		
		this.loadCover = this.loadCover.bind(this);
	}
	
	componentWillUnmount() {
		this.setState({media: false});
		this._ismounted = false;
	}
	
	componentDidMount() {
		this._ismounted = true;
	}
	
	componentWillReceiveProps(nextProps) {
		var newMedia = (!this.state.media && nextProps.media) || (this.state.media.data_source !== nextProps.media.data_source || this.state.media.path !== nextProps.media.path);
		this.setState({media: nextProps.media}, () => {
			if (newMedia) {
				this.loadCover();
			}
		});
	}
	
	loadCover() {
		if (this.state.media && !StateStore.getState().showFullScreen) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/stream/" + encodeURIComponent(StateStore.getState().profile.stream.name) + "/cover?base64&thumbnail")
			.then((result) => {
				StateStore.dispatch({type: "setMediaThumb", imgThumbBlob: result});
				this.setState({imgThumbBlob: result});
			})
			.fail(() => {
				StateStore.dispatch({type: "setMediaThumb", imgThumbBlob: false});
				this.setState({imgThumbBlob: false});
			});
		}
	}
	
	buildTitle(media) {
		var title = "";
		if (!!media.tags) {
			if (media.tags.artist || media.tags.album_artist) {
				title += (media.tags.artist || media.tags.album_artist) + " - ";
			}
			title += (media.tags.title || media.name.replace(/\.[^/.]+$/, ""));
		} else {
			title += media.name.replace(/\.[^/.]+$/, "");
		}
		return title;
	}
	
	render() {
		return (
			<MediaInfo media={this.state.media} imgThumbBlob={this.state.imgThumbBlob} meta={i18n.t("player.play_now")} index={-1}/>
		);
	}
}

export default WebradioNow;
