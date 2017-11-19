import React, { Component } from 'react';
import { Col, Row, Image } from 'react-bootstrap';
import BreadCrumbMenu from './BreadCrumbMenu';
import ElementButtons from './ElementButtons';
import StateStore from '../lib/StateStore';

class BrowseHeader extends Component {	
  constructor(props) {
    super(props);
    
    this.state = {dataSource: props.dataSource, path: props.path, imgThumbBlob: false};
    
    this.loadCover = this.loadCover.bind(this);
    
    this.loadCover();
  }
  
  componentWillReceiveProps(nextProps) {
    this.setState({dataSource: nextProps.dataSource, path: nextProps.path, imgThumbBlob: false}, () => {
      this.loadCover();
    });
  }
  
  loadCover() {
    StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
    .then((result) => {
      this.setState({imgThumbBlob: result});
    })
    .fail(() => {
      this.setState({imgThumbBlob: false});
    });
  }
  
  render () {
    if (this.state.imgThumbBlob) {
      return (
        <Row>
          <Col md={6} sm={6} xs={8}>
            <BreadCrumbMenu dataSource={this.state.dataSource} path={this.state.path} />
          </Col>
          <Col md={3} sm={3} xs={3} className="text-right">
            <Image src={"data:image/jpeg;base64," + this.state.imgThumbBlob} thumbnail responsive />
          </Col>
          <Col md={3} sm={3} xs={1} className="text-right">
            <div className="text-right">
              <ElementButtons dataSource={this.state.dataSource} path={this.state.path} element={false}/>
            </div>
          </Col>
        </Row>
      );
    } else {
      return (
        <Row>
          <Col md={9} sm={9} xs={6}>
            <BreadCrumbMenu dataSource={this.state.dataSource} path={this.state.path} />
          </Col>
          <Col md={3} sm={3} xs={6} className="text-right">
            <div className="text-right">
              <ElementButtons dataSource={this.state.dataSource} path={this.state.path} element={false}/>
            </div>
          </Col>
        </Row>
      );
    }
  }
}

export default BrowseHeader;

