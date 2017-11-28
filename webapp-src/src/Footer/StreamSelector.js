import React, { Component } from 'react';
import { DropdownButton, MenuItem, Button, ButtonGroup } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';

class StreamSelector extends Component {
  constructor(props) {
    super(props);
    this.state = {streamList: props.streamList, selectedStream: props.stream};
		this.handleSelectStream = this.handleSelectStream.bind(this);
		this.handleManageStreams = this.handleManageStreams.bind(this);
		this.handleLoadStream = this.handleLoadStream.bind(this);
		this.handleDetailsStream = this.handleDetailsStream.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
    this.setState({streamList: nextProps.streamList, selectedStream: nextProps.stream});
	}
	
	handleDetailsStream() {
    StateStore.dispatch({type: "setStreamDetails", stream: this.state.selectedStream});
    StateStore.dispatch({type: "setCurrentBrowse", browse: "streamDetails"});
	}
  
  handleLoadStream() {
    StateStore.dispatch({type: "loadStream", stream: this.state.selectedStream});
  }
	
	handleManageStreams() {
    StateStore.dispatch({type: "setCurrentBrowse", browse: "manageStream"});
	}
	
	handleSelectStream(name) {
		var newStream = this.state.streamList.find((stream) => {return stream.name === name});
		if (newStream) {
			this.setState({selectedStream: newStream});
		}
	}
	
  render() {
		var currentList = [];
		this.state.streamList.forEach((stream, index) => {
			currentList.push(
				<MenuItem key={index} eventKey={index} onClick={() => this.handleSelectStream(stream.name)} data-name={stream.name} className={stream.name===this.state.selectedStream.name?"bg-success":""}>{stream.display_name||"no name"}</MenuItem>
			)
		});
    var streamName = "Select stream";
    if (this.state.selectedStream.name) {
      streamName = this.state.selectedStream.display_name||"no name";
      if (streamName.length > 10) {
        streamName = streamName.substring(0, 10) + "...";
      }
    }
		return (
			<div>
				<ButtonGroup>
					<DropdownButton title={streamName} id="streamList">
						{currentList}
						<MenuItem divider />
						<MenuItem onClick={this.handleManageStreams}>Manage Streams</MenuItem>
					</DropdownButton>
					<Button title="Load" onClick={this.handleLoadStream}>
						<FontAwesome name={"arrow-circle-right"} />
					</Button>
					<Button title="Details" onClick={this.handleDetailsStream}>
						<FontAwesome name={"eye"} />
					</Button>
				</ButtonGroup>
			</div>
		);
  }
}

export default StreamSelector;
