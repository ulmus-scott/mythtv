import { ComponentFixture, TestBed } from '@angular/core/testing';

import { FrontendStatus } from './status';

describe('Status', () => {
  let component: FrontendStatus;
  let fixture: ComponentFixture<FrontendStatus>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [FrontendStatus]
    })
    .compileComponents();

    fixture = TestBed.createComponent(FrontendStatus);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
