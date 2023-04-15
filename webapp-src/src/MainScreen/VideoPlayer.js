import React, { Component } from 'react';
import { Row, Col, Button, ButtonGroup } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class VideoPlayer extends Component {
	constructor(props) {
		super(props);
		this.state = {};
    
		this.handleList = this.handleList.bind(this);
		this.handleDownload = this.handleDownload.bind(this);
		this.handlePlayExternal = this.handlePlayExternal.bind(this);
		this.handleCopyLink = this.handleCopyLink.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({})
	}
  
  handleList() {
    StateStore.dispatch({type: "setCurrentBrowse", browse: "showStreamMediaList"});
  }
	
  handleDownload() {
    $("#file-download-anchor")[0].click();
  }
	
  handlePlayExternal() {
    $("#file-play-external-anchor")[0].click();
  }
	
  handleCopyLink() {
     navigator.clipboard.writeText(this.getStreamUrl()+"&direct")
     .then(() => {
        StateStore.getState().NotificationManager.addNotification({
          message: i18n.t("common.copy_clipboard"),
          level: 'info'
        });
     });
  }
  
  getStreamUrl() {
    let streamUrl = "";
    if (StateStore.getState().profile.stream.icecast) {
      streamUrl = StateStore.getState().serverConfig.icecast_remote_address + "/" + StateStore.getState().profile.stream.name;
    } else {
      var randStr = Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5);
      var indexStr = StateStore.getState().profile.stream.webradio?"":("&index="+StateStore.getState().profile.jukeboxIndex);
      streamUrl = StateStore.getState().taliesinApiUrl + "/stream/" + StateStore.getState().profile.stream.name + "?rand=" + randStr + indexStr;
    }
    return streamUrl;
  }
	
	render() {
    let streamUrl = "";
    if (StateStore.getState().profile.stream.icecast) {
      streamUrl = StateStore.getState().serverConfig.icecast_remote_address + "/" + StateStore.getState().profile.stream.name;
    } else {
      var randStr = Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5);
      var indexStr = StateStore.getState().profile.stream.webradio?"":("&index="+StateStore.getState().profile.jukeboxIndex);
      streamUrl = StateStore.getState().taliesinApiUrl + "/stream/" + StateStore.getState().profile.stream.name + "?rand=" + randStr + indexStr;
    }
		return (
      <div>
        <Row>
          <Col md={12} sm={12} xs={12}>
						<ButtonGroup className="space-after">
							<Button title={i18n.t("common.list")} onClick={this.handleList}>
								<FontAwesome name="list" />
							</Button>
							<Button title={i18n.t("common.download")} onClick={this.handleDownload}>
								<FontAwesome name="download" />
							</Button>
							<Button title={i18n.t("common.external")} onClick={this.handlePlayExternal}>
								<FontAwesome name="external-link" />
							</Button>
							<Button title={i18n.t("common.copy_link")} onClick={this.handleCopyLink}>
								<FontAwesome name="copy" />
							</Button>
						</ButtonGroup>
          </Col>
        </Row>
        <video className="img-responsive" autoPlay={true} preload="auto" controls>
          <source src={streamUrl + "&direct"} /> // TODO Remove when video transcode is complete
          Your browser does not support the video tag.
        </video>
        <a href={(streamUrl+"&download")}
           style={{display: "none"}}
           target="_blank"
           id={"file-download-anchor"}>
          {i18n.t("common.download")}
        </a>
        <a href={(streamUrl+"&direct")}
           style={{display: "none"}}
           target="_blank"
           id={"file-play-external-anchor"}>
          {i18n.t("common.external")}
        </a>
      </div>
		);
	}
}

export default VideoPlayer;
