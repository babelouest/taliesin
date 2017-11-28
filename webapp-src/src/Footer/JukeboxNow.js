import React, { Component } from 'react';
import StateStore from '../lib/StateStore';
import MediaInfo from './MediaInfo';

class JukeboxNow extends Component {
  constructor(props) {
    super(props);
    this.state = {
			folded: props.folded,
			imgThumbBlob: false, 
			media: props.media,
			index: props.index
		};
    this.loadCover();
		
	}
	
	componentWillUnmount() {
		this.setState({media: false});
	}
	
	componentWillReceiveProps(nextProps) {
		var newMedia = (this.state.media.data_source !== nextProps.media.data_source || this.state.media.path !== nextProps.media.path);
		this.setState({folded: nextProps.folded, media: nextProps.media, index: nextProps.index}, () => {
			if (newMedia) {
				this.loadCover();
			}
		});
	}
	
	loadCover() {
		if (this.state.media.data_source && this.state.media.path && !StateStore.getState().showFullScreen) {
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
      <MediaInfo media={this.state.media} imgThumbBlob={this.state.imgThumbBlob} meta="Jukebox" folded={this.state.folded} index={this.state.index} total={StateStore.getState().profile.stream.elements}/>
		);
  }
}

export default JukeboxNow;
