import React, { Component } from 'react';
import StateStore from '../lib/StateStore';
import MediaInfo from './MediaInfo';
import i18n from '../lib/i18n';

class WebradioNow extends Component {
	constructor(props) {
		super(props);
		this.state = {
			folded: props.folded,
			imgThumbBlob: false, 
			media: StateStore.getState().profile.mediaNow,
		};
		this.loadCover();
		this.setPageTitle();

		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "showFullScreen") {
				if (!StateStore.getState().showFullScreen) {
					this.loadCover();
				}
			}
		});
		
		this.loadCover = this.loadCover.bind(this);
		this.setPageTitle = this.setPageTitle.bind(this);
	}
	
	componentWillUnmount() {
		this.setState({media: false});
		this._ismounted = false;
	}
	
	componentDidMount() {
		this._ismounted = true;
	}
	
	componentWillReceiveProps(nextProps) {
		var newMedia = (this.state.media.data_source !== nextProps.media.data_source || this.state.media.path !== nextProps.media.path);
		this.setState({folded: nextProps.folded, media: nextProps.media}, () => {
			if (newMedia) {
				this.loadCover();
				this.setPageTitle();
			}
		});
	}
	
	loadCover() {
		if (this.state.media && !StateStore.getState().showFullScreen) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/stream/" + encodeURIComponent(StateStore.getState().profile.stream.name) + "/cover?base64&thumbnail")
			.then((result) => {
				this.setState({imgThumbBlob: result});
			})
			.fail(() => {
				this.setState({imgThumbBlob: false});
			});
		}
	}
	
	setPageTitle() {
		if (this._ismounted) {
			if (this.state.media) {
				document.title = this.buildTitle(this.state.media) + " - Taliesin"
			} else {
				document.title = "Taliesin";
			}
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
			<MediaInfo media={this.state.media} imgThumbBlob={this.state.imgThumbBlob} meta={i18n.t("player.play_now")} folded={this.state.folded} index={-1}/>
		);
	}
}

export default WebradioNow;
