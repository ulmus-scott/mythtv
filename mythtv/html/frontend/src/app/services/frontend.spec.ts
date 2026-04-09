import { TestBed } from '@angular/core/testing';

import { FrontEndStatus } from './frontend.service';

describe('FrontEndStatus', () => {
  let service: FrontEndStatus;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(FrontEndStatus);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
