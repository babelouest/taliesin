import React, { Component } from 'react';
import FontAwesome from 'react-fontawesome';
import { DropdownButton, MenuItem, ButtonGroup, Button } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class ElementButtons extends Component {
  constructor(props) {
    super(props);
		this.state = {dataSource: props.dataSource, path: props.path, element: props.element};

		this.playElement = this.playElement.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
    this.setState({dataSource: nextProps.dataSource, path: nextProps.path, element: nextProps.element});
	}
  
  playElement() {
    StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path + "/" + this.state.element.name).replace(/#/g, "%23") + "?jukebox&recursive")
    .then((result) => {
      var streamList = StateStore.getState().streamList;
      streamList.push(result);
      StateStore.dispatch({type: "setStreamList", streamList: streamList});
      StateStore.dispatch({type: "loadStream", stream: result});
    })
    .fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error Play',
				level: 'error'
			});
    });
  }
	
	render() {
    return (
			<div>
				<ButtonGroup className="hidden-xs">
					<Button onClick={this.playElement}>
						<FontAwesome name={"play"} />
					</Button>
					<Button>
						<FontAwesome name={"play"} />&nbsp;
						<FontAwesome name={"cog"} />
					</Button>
					<Button>
						<FontAwesome name={"eye"} />
					</Button>
					<DropdownButton id={"add"-this.state.element.name} pullRight title={
						<span><i className="fa fa-plus"></i></span>
					}>
						<MenuItem>
							Add to stream
						</MenuItem>
						<MenuItem>- stream 1</MenuItem>
						<MenuItem>- stream 2</MenuItem>
						<MenuItem divider />
						<MenuItem>
							Add to playlist
						</MenuItem>
						<MenuItem>- playlist 1</MenuItem>
						<MenuItem>- playlist 2</MenuItem>
					</DropdownButton>
				</ButtonGroup>
				<DropdownButton className="visible-xs" id={"xs-manage"-this.state.element.name} pullRight title={
					<span><i className="fa fa-cog"></i></span>
				}>
					<MenuItem>
						<FontAwesome name={"play"} />&nbsp;
						Play now
					</MenuItem>
					<MenuItem divider />
					<MenuItem>
						<FontAwesome name={"play"} />
						<FontAwesome name={"cog"} />&nbsp;
						Create stream
					</MenuItem>
					<MenuItem divider />
					<MenuItem>
						<FontAwesome name={"eye"} />&nbsp;
						Details
					</MenuItem>
					<MenuItem divider />
					<MenuItem>
						<FontAwesome name={"plus"} />&nbsp;
						Add to stream
					</MenuItem>
					<MenuItem>- stream 1</MenuItem>
					<MenuItem>- stream 2</MenuItem>
					<MenuItem divider />
					<MenuItem>
						<FontAwesome name={"plus"} />&nbsp;
						Add to playlist
					</MenuItem>
					<MenuItem>- playlist 1</MenuItem>
					<MenuItem>- playlist 2</MenuItem>
				</DropdownButton>
			</div>
    );
	}
}

export default ElementButtons;
